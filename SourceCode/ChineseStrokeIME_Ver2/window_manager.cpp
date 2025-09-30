// window_manager.cpp - 視窗管理與繪製實作（基於原始2083行代碼的最小修復）
#include "window_manager.h"
#include "buffer_manager.h"
#include "dictionary.h"
#include "input_handler.h"
#include "config_loader.h"
#include "screen_manager.h"
#include "position_manager.h"
#include "tray_manager.h"
#include <algorithm>

// 前向宣告
extern TrayManager::TrayIconData g_trayIcon;

namespace WindowManager {

// 優化：雙緩衝繪製
void drawWithDoubleBuffering(HWND hwnd, HDC hdc, RECT rect,
                          void (*drawFunc)(HWND, HDC, GlobalState&),
                          GlobalState& state) {
    // 建立相容設備內容與點陣圖
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    
    // 繪製到記憶體緩衝區
    drawFunc(hwnd, memDC, state);
    
    // 一次性複製到視窗
    BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);
    
    // 清理
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

bool registerWindowClasses(HINSTANCE hInstance) {
    // 註冊主視窗類別
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"IME_MAIN";
    wc.hbrBackground = CreateSolidBrush(RGB(240,240,240));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc)) {
        return false;
    }
    
    // 註冊候選視窗類別
    WNDCLASSW wc2 = {0};
    wc2.lpfnWndProc = CandProc;
    wc2.hInstance = hInstance;
    wc2.lpszClassName = L"IME_CAND";
    wc2.hbrBackground = NULL;
    wc2.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc2)) {
        return false;
    }
    
    // 註冊暫放視窗類別
    WNDCLASSW wc3 = {0};
    wc3.lpfnWndProc = BufferProc;
    wc3.hInstance = hInstance;
    wc3.lpszClassName = L"IME_BUFFER";
    wc3.hbrBackground = CreateSolidBrush(RGB(255,255,255));
    wc3.hCursor = LoadCursor(NULL, IDC_IBEAM);
    if (!RegisterClassW(&wc3)) {
        return false;
    }
    
    return true;
}

bool createWindows(HINSTANCE hInstance, GlobalState& state) {
    // 創建主視窗
    state.hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
        L"IME_MAIN", 
        L"中文筆劃輸入法",
        WS_POPUP | WS_BORDER, 
        PositionManager::g_toolbarPos.x, 
        PositionManager::g_toolbarPos.y, 
        state.windowWidth, 
        state.windowHeight,
        NULL, NULL, hInstance, NULL
    );
    
    if (!state.hWnd) {
        return false;
    }
    
    // 創建候選視窗
    state.hCandWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
        L"IME_CAND", 
        L"",
        WS_POPUP | WS_BORDER, 
        100, 180, state.candidateWidth, state.candidateHeight,
        NULL, NULL, hInstance, NULL
    );
    
    if (!state.hCandWnd) {
        return false;
    }
    
    // 創建暫放視窗
    state.hBufferWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
        L"IME_BUFFER", 
        L"",
        WS_POPUP | WS_BORDER, 
        100, 280, FIXED_WIDTH, MIN_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (!state.hBufferWnd) {
        return false;
    }
    
    return true;
}

void showAboutDialog(HWND hwnd, bool isOptimizedUI = true) {
    if (isOptimizedUI) {
        MessageBoxW(hwnd,
            L"中文筆劃輸入法 V2.0\n\n"
			L"開發者: Claude AI\n"
            L"測試專員: 山崎大叔\n\n"	
			L"\n"	
            L"✨ 主要特色\n"
			L"✓ 免安裝、免Admin權限\n"
			L"✓ 隨身攜帶、即開即用\n"
			L"✓ 暫放模式功能\n\n"
			L"⌨️ 基本操作\n"
			L"• U I O J K：基本筆劃輸入\n"
			L"• 1-9數字鍵：選擇候選字\n"
			L"• Shift鍵：切換中英文模式\n"
			L"• 右鍵托盤圖示：快捷選單\n\n"
			L"感謝您的使用！", 
			L"關於 - 中文筆劃輸入法",
            MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(hwnd,
            L"中文筆劃輸入法 V2.0\n\n"
            L"開發者: Claude AI\n"
            L"測試專員: 山崎大叔\n"						
            L"V2.0 新增功能:\n" 
			L"支援暫放模式，避免防毒軟件阻擋文字輸入\n"
			L"加入更多Config自定配置\n"
			L"感謝您的使用！",
            L"關於 - 中文筆劃輸入法", 
            MB_OK | MB_ICONINFORMATION);
    }
}



void drawCloseButton(HDC hdc, RECT windowRect, GlobalState& state) {
    int buttonSize = 18;
    state.closeButtonRect.left = windowRect.right - buttonSize - 3;
    state.closeButtonRect.top = windowRect.top + 3;
    state.closeButtonRect.right = state.closeButtonRect.left + buttonSize;
    state.closeButtonRect.bottom = state.closeButtonRect.top + buttonSize;
    
    COLORREF buttonColor = state.closeButtonHover ? state.closeButtonHoverColor : state.closeButtonColor;
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &state.closeButtonRect, hBrush);
    DeleteObject(hBrush);
    
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255,255,255));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    
    int margin = 3;
    MoveToEx(hdc, state.closeButtonRect.left + margin, state.closeButtonRect.top + margin, NULL);
    LineTo(hdc, state.closeButtonRect.right - margin, state.closeButtonRect.bottom - margin);
    MoveToEx(hdc, state.closeButtonRect.right - margin, state.closeButtonRect.top + margin, NULL);
    LineTo(hdc, state.closeButtonRect.left + margin, state.closeButtonRect.bottom - margin);
    
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void drawModeButton(HDC hdc, RECT windowRect, GlobalState& state) {
    int buttonSize = 18;
    state.modeButtonRect.left = windowRect.right - buttonSize - 25;
    state.modeButtonRect.top = windowRect.top + 3;
    state.modeButtonRect.right = state.modeButtonRect.left + buttonSize;
    state.modeButtonRect.bottom = state.modeButtonRect.top + buttonSize;
    
    COLORREF buttonColor = state.modeButtonHover ? state.modeButtonHoverColor : state.modeButtonColor;
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &state.modeButtonRect, hBrush);
    DeleteObject(hBrush);
    
    HFONT hSmallFont = CreateFontW(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                 DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
    
    SetTextColor(hdc, RGB(255,255,255));
    SetBkMode(hdc, TRANSPARENT);
    
    std::wstring buttonText = state.chineseMode ? L"中" : L"英";
    TextOutW(hdc, state.modeButtonRect.left + 4, state.modeButtonRect.top + 2, 
            buttonText.c_str(), (int)buttonText.size());
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hSmallFont);
}

void drawCreditsButton(HDC hdc, RECT windowRect, GlobalState& state) {
    int buttonSize = 18;
    state.creditsButtonRect.left = windowRect.right - buttonSize - 47;
    state.creditsButtonRect.top = windowRect.top + 3;
    state.creditsButtonRect.right = state.creditsButtonRect.left + buttonSize;
    state.creditsButtonRect.bottom = state.creditsButtonRect.top + buttonSize;
    
    COLORREF buttonColor = state.creditsButtonHover ? state.creditsButtonHoverColor : state.creditsButtonColor;
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &state.creditsButtonRect, hBrush);
    DeleteObject(hBrush);
    
    HFONT hSmallFont = CreateFontW(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
    
    SetTextColor(hdc, RGB(255,255,255));
    SetBkMode(hdc, TRANSPARENT);
    
    TextOutW(hdc, state.creditsButtonRect.left + 6, state.creditsButtonRect.top + 2, L"？", 1);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hSmallFont);
}

void drawRefreshButton(HDC hdc, RECT windowRect, GlobalState& state) {
    int buttonSize = 18;
    state.refreshButtonRect.left = windowRect.right - buttonSize - 69;
    state.refreshButtonRect.top = windowRect.top + 3;
    state.refreshButtonRect.right = state.refreshButtonRect.left + buttonSize;
    state.refreshButtonRect.bottom = state.refreshButtonRect.top + buttonSize;
    
    COLORREF buttonColor = state.refreshButtonHover ? state.refreshButtonHoverColor : state.refreshButtonColor;
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &state.refreshButtonRect, hBrush);
    DeleteObject(hBrush);
    
    HFONT hSmallFont = CreateFontW(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
    
    SetTextColor(hdc, RGB(255,255,255));
    SetBkMode(hdc, TRANSPARENT);
    
    TextOutW(hdc, state.refreshButtonRect.left + 5, state.refreshButtonRect.top + 2, L"↻", 1);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hSmallFont);
}

void drawBufferButton(HDC hdc, RECT windowRect, GlobalState& state) {
    int buttonSize = 18;
    state.bufferButtonRect.left = windowRect.right - buttonSize - 91;
    state.bufferButtonRect.top = windowRect.top + 3;
    state.bufferButtonRect.right = state.bufferButtonRect.left + buttonSize;
    state.bufferButtonRect.bottom = state.bufferButtonRect.top + buttonSize;
    
    COLORREF buttonColor;
    if (state.bufferMode) {
        buttonColor = RGB(255, 165, 0);
    } else {
        buttonColor = state.bufferButtonHover ? RGB(120, 120, 120) : RGB(100, 100, 100);
    }
    
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &state.bufferButtonRect, hBrush);
    DeleteObject(hBrush);
    
    HFONT hSmallFont = CreateFontW(10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                 DEFAULT_PITCH | FF_DONTCARE, L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
    
    SetTextColor(hdc, RGB(255,255,255));
    SetBkMode(hdc, TRANSPARENT);
    
    TextOutW(hdc, state.bufferButtonRect.left + 5, state.bufferButtonRect.top + 2, L"緩", 1);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hSmallFont);
}

void drawMain(HWND hwnd, HDC hdc, GlobalState& state) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    SetBkMode(hdc, TRANSPARENT);
    
    HBRUSH hBg = CreateSolidBrush(state.bgColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    
    drawCloseButton(hdc, rc, state);
    drawModeButton(hdc, rc, state);
    drawCreditsButton(hdc, rc, state);
    drawRefreshButton(hdc, rc, state);
    drawBufferButton(hdc, rc, state);
    
    HFONT hFont = CreateFontW(state.fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                            DEFAULT_PITCH | FF_DONTCARE, state.fontName.c_str());
    HFONT hOld = (HFONT)SelectObject(hdc, hFont);
    
    SetTextColor(hdc, state.chineseMode ? RGB(0,150,0) : RGB(0,100,200));
    
    std::wstring mode = state.chineseMode ? L"中文+全形" : L"英文+半形";
    if (state.bufferMode) mode += L" [暫放]";
    TextOutW(hdc, 10, 5, mode.c_str(), (int)mode.size());
    
    SetTextColor(hdc, state.inputError ? state.errorColor : state.textColor);
    
    std::wstring display;
    if (state.showPunctMenu) {
        display = L"標點符號選單（P鍵開啟）";
    } else if (state.input.empty()) {
        display = state.chineseMode ? L"中文筆劃輸入法（UIOJKL），P=標點，Shift=切換" : L"英文直接輸入，Shift=切換";
        if (state.bufferMode) display += L"，緩=暫放模式";
    } else if (state.inputError) {
        display = L"輸入不當：" + state.input;
    } else {
        display = state.input;
    }
    TextOutW(hdc, 100, 5, display.c_str(), (int)display.size());
    
    SetTextColor(hdc, RGB(100, 100, 100));
    TextOutW(hdc, 10, 35, state.statusInfo.c_str(), (int)state.statusInfo.size());
    
    // 版本資訊
    std::wstring info = L"多螢幕整合版 1.0 | 記憶詞庫：" + 
                     std::to_wstring(state.wordFreq.size()) + L" | 字典：" + 
                     std::to_wstring(state.dictSize);
    
    if (state.bufferMode) {
        info += L" | 暫放：" + std::to_wstring(state.bufferText.length()) + L"字";
    }
    
    TextOutW(hdc, 10, 55, info.c_str(), (int)info.size());
    
    SelectObject(hdc, hOld);
    DeleteObject(hFont);
}

void drawCandidate(HWND hwnd, HDC hdc, const GlobalState& state) {
    if (state.candidates.empty()) return;
    
    RECT rc;
    GetClientRect(hwnd, &rc);
    
    // 背景和邊框
    HBRUSH hBg = CreateSolidBrush(state.candidateBackgroundColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(180,180,180));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, rc.right, rc.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT hFont = CreateFontW(
        state.candidateFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        state.candidateFontName.c_str()
    );
    HFONT hOld = (HFONT)SelectObject(hdc, hFont);
    
    int lineHeight = state.candidateFontSize + 6;
    int startIndex = state.currentPage * CANDIDATES_PER_PAGE;
    int endIndex = std::min(startIndex + CANDIDATES_PER_PAGE, (int)state.candidates.size());
    
    // 繪製候選字列表
    for (int i = 0; i < endIndex - startIndex; ++i) {
        int actualIndex = startIndex + i;
        
        if (i == state.selected) {
            RECT bgRect = {8, 8 + i * lineHeight, rc.right - 8, 8 + (i + 1) * lineHeight};
            HBRUSH hBrush = CreateSolidBrush(state.selectedCandidateBackgroundColor);
            FillRect(hdc, &bgRect, hBrush);
            DeleteObject(hBrush);
            SetTextColor(hdc, state.selectedCandidateTextColor);
        } else {
            SetTextColor(hdc, state.candidateTextColor);
        }
        
        std::wstring txt;
        if (state.showPunctMenu) {
            txt = std::to_wstring(i+1) + L". " + state.candidates[actualIndex];
        } else {
            std::wstring codeInfo = L" [" + state.candidateCodes[actualIndex] + L"]";
            std::wstring detailInfo = L"";
            
            if (state.wordFreq.find(state.candidates[actualIndex]) != state.wordFreq.end()) {
                const WordInfo& info = state.wordFreq.at(state.candidates[actualIndex]);
                detailInfo = info.isPermanent ? L" ★" : L" (" + std::to_wstring(info.frequency) + L")";
            }
            
            txt = std::to_wstring(i+1) + L". " + state.candidates[actualIndex] + detailInfo + codeInfo;
        }
        
        TextOutW(hdc, 15, 10 + i * lineHeight, txt.c_str(), (int)txt.size());
    }
    
    // 繪製翻頁控制區域（如果有多頁）
    if (state.totalPages > 1) {
        int controlY = 10 + CANDIDATES_PER_PAGE * lineHeight + 25;
        int buttonSize = 20;
        int buttonY = controlY;
        
        // 分隔線
        HPEN hSepPen = CreatePen(PS_SOLID, 1, RGB(200,200,200));
        HPEN hOldSepPen = (HPEN)SelectObject(hdc, hSepPen);
        MoveToEx(hdc, 10, controlY - 4, NULL);
        LineTo(hdc, rc.right - 10, controlY - 4);
        SelectObject(hdc, hOldSepPen);
        DeleteObject(hSepPen);
        
        // 向上翻頁按鈕
        const_cast<GlobalState&>(state).prevPageButtonRect = {10, buttonY, 10 + buttonSize, buttonY + buttonSize};
        COLORREF prevBtnColor = (state.currentPage > 0) ? 
            (state.prevPageButtonHover ? RGB(180,180,180) : RGB(220,220,220)) : RGB(240,240,240);
        HBRUSH hPrevBrush = CreateSolidBrush(prevBtnColor);
        FillRect(hdc, &const_cast<GlobalState&>(state).prevPageButtonRect, hPrevBrush);
        DeleteObject(hPrevBrush);
        
        // 向上按鈕邊框
        HPEN hBtnPen = CreatePen(PS_SOLID, 1, RGB(160,160,160));
        HPEN hOldBtnPen = (HPEN)SelectObject(hdc, hBtnPen);
        SelectObject(hdc, GetStockObject(NULL_BRUSH));
        Rectangle(hdc, const_cast<GlobalState&>(state).prevPageButtonRect.left, 
                 const_cast<GlobalState&>(state).prevPageButtonRect.top,
                 const_cast<GlobalState&>(state).prevPageButtonRect.right, 
                 const_cast<GlobalState&>(state).prevPageButtonRect.bottom);
        
        // 向上箭頭
        SetTextColor(hdc, (state.currentPage > 0) ? RGB(60,60,60) : RGB(180,180,180));
        RECT upArrowRect = const_cast<GlobalState&>(state).prevPageButtonRect;
        DrawTextW(hdc, L"▲", -1, &upArrowRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // 向下翻頁按鈕
        const_cast<GlobalState&>(state).nextPageButtonRect = {35, buttonY, 35 + buttonSize, buttonY + buttonSize};
        COLORREF nextBtnColor = (state.currentPage < state.totalPages - 1) ? 
            (state.nextPageButtonHover ? RGB(180,180,180) : RGB(220,220,220)) : RGB(240,240,240);
        HBRUSH hNextBrush = CreateSolidBrush(nextBtnColor);
        FillRect(hdc, &const_cast<GlobalState&>(state).nextPageButtonRect, hNextBrush);
        DeleteObject(hNextBrush);
        
        // 向下按鈕邊框
        Rectangle(hdc, const_cast<GlobalState&>(state).nextPageButtonRect.left, 
                 const_cast<GlobalState&>(state).nextPageButtonRect.top,
                 const_cast<GlobalState&>(state).nextPageButtonRect.right, 
                 const_cast<GlobalState&>(state).nextPageButtonRect.bottom);
        
        SelectObject(hdc, hOldBtnPen);
        DeleteObject(hBtnPen);
        
        // 向下箭頭
        SetTextColor(hdc, (state.currentPage < state.totalPages - 1) ? RGB(60,60,60) : RGB(180,180,180));
        RECT downArrowRect = const_cast<GlobalState&>(state).nextPageButtonRect;
        DrawTextW(hdc, L"▼", -1, &downArrowRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        // 頁數和統計信息
        const_cast<GlobalState&>(state).pageInfoRect = {65, buttonY, rc.right - 10, buttonY + buttonSize};
        SetTextColor(hdc, RGB(100, 100, 100));
        std::wstring pageInfo = std::to_wstring(state.currentPage + 1) + L"/" + 
                               std::to_wstring(state.totalPages) + L" (共" + 
                               std::to_wstring(state.candidates.size()) + L"個)";
        
        RECT pageTextRect = const_cast<GlobalState&>(state).pageInfoRect;
        DrawTextW(hdc, pageInfo.c_str(), -1, &pageTextRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
    
    SelectObject(hdc, hOld);
    DeleteObject(hFont);
}

int calculateOptimalWindowWidth(const GlobalState& state) {
    // 如果配置檔案有設定，優先使用配置檔案的寬度
    int configWidth = state.candidateWidth;
    if (configWidth >= 200 && configWidth <= 1000) {
        return configWidth;
    }
    
    // 否則動態計算
    int baseWidth = 300;
    
    // 根據候選字內容調整所需寬度
    if (!state.candidates.empty()) {
        int maxContentWidth = 0;
        for (size_t i = 0; i < std::min(state.candidates.size(), (size_t)CANDIDATES_PER_PAGE); ++i) {
            int contentWidth = 60;
            contentWidth += (int)state.candidates[i].length() * 18;
            
            if (i < state.candidateCodes.size()) {
                contentWidth += 30 + (int)state.candidateCodes[i].length() * 10;
            }
            
            if (state.wordFreq.find(state.candidates[i]) != state.wordFreq.end()) {
                contentWidth += 30;
            }
            
            maxContentWidth = std::max(maxContentWidth, contentWidth);
        }
        baseWidth = std::max(baseWidth, maxContentWidth + 20);
    }
    
    if (state.totalPages > 1) {
        baseWidth = std::max(baseWidth, 350);
    }
    
    // 限制最大寬度，但不小於配置檔案設定
    return std::max(configWidth, std::min(baseWidth, 600));
}

int calculateCandidateWindowHeight(const GlobalState& state) {
    if (!state.showCand || state.candidates.empty()) return 0;
    
    int lineHeight = state.candidateFontSize + 8;
    int contentLines = std::min(CANDIDATES_PER_PAGE, (int)state.candidates.size());
    int baseHeight = 16; // 上下邊距
    
    // 候選字列表高度
    baseHeight += contentLines * lineHeight;
    
    // 翻頁控制區域高度（如果有多頁）
    if (state.totalPages > 1) {
        baseHeight += 50; // 分隔線 + 翻頁按鈕區域高度
    }
    
    return baseHeight;
}

// 修復：改進視窗定位邏輯，確保字碼和候選字視窗同步
void positionWindowsOptimized(GlobalState& state) {
    if (!state.isInputting) return; // 不在輸入狀態時直接返回
    
    ScreenManager::updateMonitorInfo();
    
    // 修改：即使沒有候選字也要定位字碼視窗
    if (!state.showCand) {
        positionInputWindow(state); // 定位字碼視窗
        return;
    }
    
    int candWidth = calculateOptimalWindowWidth(state);
    int candHeight = calculateCandidateWindowHeight(state);
    
    POINT basePos;
    
    // 如果用戶設定了固定位置，使用用戶位置；否則跟隨滑鼠
    if (PositionManager::g_useUserPosition && PositionManager::g_userCandPos.isValid) {
        basePos.x = PositionManager::g_userCandPos.x;
        basePos.y = PositionManager::g_userCandPos.y;
    } else {
        // 跟隨滑鼠位置
        basePos = PositionManager::getCurrentMousePosition();
        
        ScreenManager::MonitorInfo currentMonitor = 
            ScreenManager::getMonitorFromPoint(basePos);
        RECT screenRect = currentMonitor.workArea;
        
        // 為字碼視窗預留空間
        int totalHeight = candHeight + INPUT_WINDOW_HEIGHT + 5;
        
        // 調整位置確保整個視窗組合在螢幕範圍內
        if (basePos.x + candWidth > screenRect.right - 10) {
            basePos.x = screenRect.right - candWidth - 10;
        }
        if (basePos.x < screenRect.left + 10) {
            basePos.x = screenRect.left + 10;
        }
        
        if (basePos.y + totalHeight > screenRect.bottom - 30) {
            int newY = basePos.y - totalHeight - PositionManager::g_verticalOffset;
            if (newY >= screenRect.top + 10) {
                basePos.y = newY + INPUT_WINDOW_HEIGHT + 5; // 調整候選字視窗位置
            } else {
                basePos.y = screenRect.top + 10 + INPUT_WINDOW_HEIGHT + 5;
            }
        }
        if (basePos.y < screenRect.top + INPUT_WINDOW_HEIGHT + 15) {
            basePos.y = screenRect.top + INPUT_WINDOW_HEIGHT + 15;
        }
    }
    
    // 定位候選字視窗
    if (state.hCandWnd && state.showCand && candHeight > 0) {
        SetWindowPos(state.hCandWnd, HWND_TOPMOST,
                    basePos.x, basePos.y,
                    candWidth, candHeight,
                    SWP_NOACTIVATE | SWP_SHOWWINDOW);
        InvalidateRect(state.hCandWnd, nullptr, TRUE);
    }
    
    // 關鍵修改：候選字視窗定位後，立即定位字碼視窗
    positionInputWindow(state);
}

void positionMainWindow(GlobalState& state) {
    // 主視窗位置
    SetWindowPos(state.hWnd, NULL, 
                PositionManager::g_toolbarPos.x, 
                PositionManager::g_toolbarPos.y,
                state.windowWidth, state.windowHeight,
                SWP_NOZORDER);
}

bool isPointInCloseButton(int x, int y, const GlobalState& state) {
    return x >= state.closeButtonRect.left && x <= state.closeButtonRect.right &&
           y >= state.closeButtonRect.top && y <= state.closeButtonRect.bottom;
}

bool isPointInModeButton(int x, int y, const GlobalState& state) {
    return x >= state.modeButtonRect.left && x <= state.modeButtonRect.right &&
           y >= state.modeButtonRect.top && y <= state.modeButtonRect.bottom;
}

bool isPointInCreditsButton(int x, int y, const GlobalState& state) {
    return x >= state.creditsButtonRect.left && x <= state.creditsButtonRect.right &&
           y >= state.creditsButtonRect.top && y <= state.creditsButtonRect.bottom;
}

bool isPointInRefreshButton(int x, int y, const GlobalState& state) {
    return x >= state.refreshButtonRect.left && x <= state.refreshButtonRect.right &&
           y >= state.refreshButtonRect.top && y <= state.refreshButtonRect.bottom;
}

bool isPointInBufferButton(int x, int y, const GlobalState& state) {
    return x >= state.bufferButtonRect.left && x <= state.bufferButtonRect.right &&
           y >= state.bufferButtonRect.top && y <= state.bufferButtonRect.bottom;
}

bool isPointInSendButton(int x, int y, const GlobalState& state) {
    return x >= state.sendButtonRect.left && x <= state.sendButtonRect.right &&
           y >= state.sendButtonRect.top && y <= state.sendButtonRect.bottom;
}

bool isPointInClearButton(int x, int y, const GlobalState& state) {
    return x >= state.clearButtonRect.left && x <= state.clearButtonRect.right &&
           y >= state.clearButtonRect.top && y <= state.clearButtonRect.bottom;
}

bool isPointInSaveButton(int x, int y, const GlobalState& state) {
    return x >= state.saveButtonRect.left && x <= state.saveButtonRect.right &&
           y >= state.saveButtonRect.top && y <= state.saveButtonRect.bottom;
}

bool isPointInPrevPageButton(int x, int y, const GlobalState& state) {
    return x >= state.prevPageButtonRect.left && x <= state.prevPageButtonRect.right &&
           y >= state.prevPageButtonRect.top && y <= state.prevPageButtonRect.bottom;
}

bool isPointInNextPageButton(int x, int y, const GlobalState& state) {
    return x >= state.nextPageButtonRect.left && x <= state.nextPageButtonRect.right &&
           y >= state.nextPageButtonRect.top && y <= state.nextPageButtonRect.bottom;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_DESTROY:
            // 清理資源
            if (g_hKeyboardHook) {
                UnhookWindowsHookEx(g_hKeyboardHook);
                g_hKeyboardHook = NULL;
            }
            
            // 儲存用戶設定
            Dictionary::saveUserDict(g_state);
            PositionManager::savePositions(g_state);
            
            // 移除系統托盤圖示
            TrayManager::removeTrayIcon(&g_trayIcon);
            
            PostQuitMessage(0);
            return 0;
            
        case WM_DISPLAYCHANGE: 
            // 延遲處理，等待系統完成切換
            static UINT_PTR delayTimerId = 0;
            if (delayTimerId) {
                KillTimer(hwnd, delayTimerId);
            }
            delayTimerId = SetTimer(hwnd, 997, 500, NULL);
            return 0;

        case WM_USER+200: 
            // 系統托盤消息
            TrayManager::processTrayMessage(hwnd, lp, g_state);
            return 0;
            
        case WM_USER+100: {
            DWORD key = (DWORD)wp;
            if (g_state.chineseMode) {
                if (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L' || key == 'P' ||
                    key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0) {
                    InputHandler::processStroke(g_state, key);
                    return 0;
                }
                if (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 ||
                    key == VK_OEM_1 || key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 ||
                    key == VK_SPACE || key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||
                    (key == '1' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '2' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (key == '3' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '4' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (key == '5' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '6' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (key == '7' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '8' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (key == '9' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '0' && (GetKeyState(VK_SHIFT) & 0x8000))) {
                    InputHandler::processPunctuator(g_state, key);
                    return 0;
                }
            }
            if (key == VK_DOWN) {
                Dictionary::changePage(g_state, 1);
                return 0;
            }
            if (key == VK_UP) {
                Dictionary::changePage(g_state, -1);
                return 0;
            }
            if (key >= '1' && key <= '9') {
                Dictionary::selectCandidate(g_state, key - '1');
                return 0;
            }
            if (key == VK_BACK) {
                if (!g_state.input.empty()) {
                    g_state.input.pop_back();
                    Dictionary::updateCandidates(g_state);
                    InvalidateRect(hwnd, nullptr, TRUE);
                }
                return 0;
            }
            if (key == VK_SPACE) {
                Dictionary::selectCandidate(g_state, 0);
                return 0;
            }
            if (key == VK_RETURN) {
                InputHandler::handleEnterKeySmartly(g_state);
                return 0;
            }
            if (key == VK_ESCAPE) {
                g_state.input.clear();
                g_state.candidates.clear();
                g_state.candidateCodes.clear();
                g_state.showCand = false;
                g_state.isInputting = false;
                g_state.inputError = false;
                g_state.showPunctMenu = false;
                if (g_state.hCandWnd) ShowWindow(g_state.hCandWnd, SW_HIDE);
                if (g_state.hInputWnd) ShowWindow(g_state.hInputWnd, SW_HIDE); // 修復：ESC時也隱藏字碼視窗
                Utils::updateStatus(g_state, L"輸入已取消");
                InvalidateRect(hwnd, nullptr, TRUE);
                return 0;
            }
            break;
        }
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // 使用雙緩衝繪製
            drawMain(hwnd, hdc, g_state);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
            
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lp); 
            int y = HIWORD(lp);
            
            if (isPointInCloseButton(x, y, g_state)) {
                if (MessageBoxW(hwnd, L"確定要關閉輸入法嗎？", L"確認關閉", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                }
                return 0;
            }
            
            if (isPointInModeButton(x, y, g_state)) {
                InputHandler::toggleInputMode(g_state);
                return 0;
            }
            
            if (isPointInCreditsButton(x, y, g_state)) {
				showAboutDialog(hwnd, false);
                return 0;
            }
            
            if (isPointInRefreshButton(x, y, g_state)) {
                ConfigLoader::refreshConfigs(g_state);
                return 0;
            }
            
            if (isPointInBufferButton(x, y, g_state)) {
                BufferManager::toggleBufferMode(g_state);
                return 0;
            }
            
            g_state.isDragging = true;
            SetCapture(hwnd);
            g_state.dragStartPoint.x = x;
            g_state.dragStartPoint.y = y;
            return 0;
        }
            
        case WM_MOUSEMOVE: {
            if (g_state.isDragging) {
                POINT pt;
                GetCursorPos(&pt);
                
                // 計算新位置
                int newX = pt.x - g_state.dragStartPoint.x;
                int newY = pt.y - g_state.dragStartPoint.y;
                
                // 更新工具列位置
                SetWindowPos(hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                
                // 更新全域位置紀錄
                PositionManager::g_toolbarPos.x = newX;
                PositionManager::g_toolbarPos.y = newY;
            }
            
            int x = LOWORD(lp); 
            int y = HIWORD(lp);
            bool needRedraw = false;
            
            bool newCloseHover = isPointInCloseButton(x, y, g_state);
            bool newModeHover = isPointInModeButton(x, y, g_state);
            bool newCreditsHover = isPointInCreditsButton(x, y, g_state);
            bool newRefreshHover = isPointInRefreshButton(x, y, g_state);
            bool newBufferHover = isPointInBufferButton(x, y, g_state);
            
            if (newCloseHover != g_state.closeButtonHover || 
                newModeHover != g_state.modeButtonHover || 
                newCreditsHover != g_state.creditsButtonHover || 
                newRefreshHover != g_state.refreshButtonHover || 
                newBufferHover != g_state.bufferButtonHover) {
                needRedraw = true;
            }
            
            g_state.closeButtonHover = newCloseHover;
            g_state.modeButtonHover = newModeHover;
            g_state.creditsButtonHover = newCreditsHover;
            g_state.refreshButtonHover = newRefreshHover;
            g_state.bufferButtonHover = newBufferHover;
            
            if (needRedraw) {
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            return 0;
        }
            
        case WM_LBUTTONUP: {
            if (g_state.isDragging) {
                g_state.isDragging = false;
                ReleaseCapture();
                
                // 儲存新位置
                PositionManager::savePositions(g_state);
                return 0;
            }
            break;
        }
            
        case WM_COMMAND: {
            switch (LOWORD(wp)) {
                // 托盤選單命令處理
                case 2001: // 顯示輸入法
                    TrayManager::showFromTray(hwnd, &g_trayIcon);
                    break;
                case 2002: // 重置為滑鼠跟隨
                    PositionManager::g_useUserPosition = false;
                    PositionManager::savePositions(g_state);
                    Utils::updateStatus(g_state, L"已重置為滑鼠跟隨模式");
                    break;
                case 2003: // 關閉輸入法
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                    break;
                case 2004: // 切換模式
                    InputHandler::toggleInputMode(g_state);
                    break;
                case 2005: // 標點符號選單
                    InputHandler::showPunctMenu(g_state);
                    break;
                case 2006: // 重新載入配置
                    ConfigLoader::refreshConfigs(g_state);
                    break;
                case 2007: // 取消固定位置
                    PositionManager::g_useUserPosition = false;
                    PositionManager::savePositions(g_state);
                    Utils::updateStatus(g_state, L"已取消固定位置");
                    break;
                case 2008: // 關於
                    MessageBoxW(hwnd, 
                        L"中文筆劃輸入法 V2.0\n"
                        L"開發者: Claude AI\n"
                        L"測試專員: 山崎大叔\n"						
                        L"增加功能:\n" 
						L"支援暫放模式，避免防毒軟件阻擋文字輸入\n"
                        L"增加更多Config自訂配置",
                        L"關於", MB_OK | MB_ICONINFORMATION);
                    break;
                case 2009: // 重啟輸入法
                    if (MessageBoxW(hwnd, L"確定要重啟輸入法嗎？\n\n重啟後將保留所有設定和學習紀錄。", 
                        L"確認重啟", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        wchar_t exePath[MAX_PATH];
                        GetModuleFileNameW(NULL, exePath, MAX_PATH);
                        
                        STARTUPINFOW si = {0};
                        PROCESS_INFORMATION pi = {0};
                        si.cb = sizeof(STARTUPINFOW);
                        
                        if (CreateProcessW(exePath, NULL, NULL, NULL, FALSE, 
                                          0, NULL, NULL, &si, &pi)) {
                            CloseHandle(pi.hProcess);
                            CloseHandle(pi.hThread);
                            
                            Sleep(500);
                            
                            PostQuitMessage(0);
                        } else {
                            MessageBoxW(NULL, L"重啟輸入法失敗", L"錯誤", MB_OK | MB_ICONERROR);
                        }
                    }
                    break;
                case 2010: // 發送暫放文字
                    if (g_state.bufferMode) {
                        BufferManager::sendBufferContent(g_state);
                    }
                    break;
                case 2011: // 清空暫放區
                    if (g_state.bufferMode) {
                        BufferManager::clearBufferWithConfirm(g_state);
                    }
                    break;
                case 2012: // 開啟暫放模式
                    if (!g_state.bufferMode) {
                        BufferManager::toggleBufferMode(g_state);
                    }
                    break;
            }
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK CandProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;
            
        case WM_PAINT: { 
            PAINTSTRUCT ps; 
            HDC hdc = BeginPaint(hwnd, &ps); 
            drawCandidate(hwnd, hdc, g_state); 
            EndPaint(hwnd, &ps); 
            return 0; 
        }
            
        case WM_LBUTTONDOWN: { 
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            
            // ★ 優先處理翻頁按鈕點擊
            if (g_state.totalPages > 1) {
                if (isPointInPrevPageButton(x, y, g_state) && g_state.currentPage > 0) {
                    Dictionary::changePage(g_state, -1);
                    return 0;  // 重要：處理完後返回，避免繼續處理候選字點擊
                }
                
                if (isPointInNextPageButton(x, y, g_state) && g_state.currentPage < g_state.totalPages - 1) {
                    Dictionary::changePage(g_state, 1);
                    return 0;  // 重要：處理完後返回
                }
            }
            
            // 處理候選字點擊（只有當不是翻頁按鈕時）
            int lineHeight = g_state.candidateFontSize + 6;
            int idx = (y - 10) / lineHeight;
            
            // 確保點擊在候選字區域內（不在翻頁控制區域）
            int maxCandidateY = 10 + CANDIDATES_PER_PAGE * lineHeight;
            if (y < maxCandidateY && idx >= 0 && idx < CANDIDATES_PER_PAGE) {
                Dictionary::selectCandidate(g_state, idx);
            }
            
            return 0; 
        }
        
        case WM_MOUSEMOVE: {
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            
            bool needRedraw = false;
            
            if (g_state.totalPages > 1) {
                bool newPrevHover = isPointInPrevPageButton(x, y, g_state) && g_state.currentPage > 0;
                bool newNextHover = isPointInNextPageButton(x, y, g_state) && g_state.currentPage < g_state.totalPages - 1;
                
                if (newPrevHover != g_state.prevPageButtonHover || newNextHover != g_state.nextPageButtonHover) {
                    needRedraw = true;
                }
                
                g_state.prevPageButtonHover = newPrevHover;
                g_state.nextPageButtonHover = newNextHover;
                
                if (needRedraw) {
                    InvalidateRect(hwnd, nullptr, TRUE);
                }
            }
            
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wp, lp);
}

// 新增函數：繪製帶選取高亮的文字
void drawTextWithSelection(HDC hdc, RECT textArea, GlobalState& state) {
    if (state.bufferText.empty()) return;
    
    // 設定字體
    HFONT hFont = CreateFontW(state.bufferFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, state.bufferFontName.c_str());
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    SetBkMode(hdc, TRANSPARENT);
    
    int currentX = textArea.left;
    int currentY = textArea.top;
    int lineHeight = state.bufferFontSize + 2;
    
    int selStart = state.hasSelection ? std::min(state.selectionStart, state.selectionEnd) : -1;
    int selEnd = state.hasSelection ? std::max(state.selectionStart, state.selectionEnd) : -1;
    
    for (int i = 0; i < (int)state.bufferText.length(); i++) {
        wchar_t ch = state.bufferText[i];
        SIZE charSize;
        GetTextExtentPoint32W(hdc, &ch, 1, &charSize);
        
        // 檢查是否需要換行
        if (currentX + charSize.cx > textArea.right) {
            currentX = textArea.left;
            currentY += lineHeight;
            
            // 檢查是否超出顯示區域
            if (currentY + state.bufferFontSize > textArea.bottom) {
                break; // 不再繪製
            }
        }
        
        // 繪製選取背景
        if (state.hasSelection && i >= selStart && i < selEnd) {
            RECT charRect = {currentX, currentY, currentX + charSize.cx, currentY + state.bufferFontSize};
            HBRUSH hSelBrush = CreateSolidBrush(RGB(51, 153, 255)); // 藍色選取背景
            FillRect(hdc, &charRect, hSelBrush);
            DeleteObject(hSelBrush);
            
            // 選取文字使用白色
            SetTextColor(hdc, RGB(255, 255, 255));
        } else {
            // 正常文字顏色
            SetTextColor(hdc, state.bufferTextColor);
        }
        
        // 繪製字符
        TextOutW(hdc, currentX, currentY, &ch, 1);
        
        currentX += charSize.cx;
    }
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

// 暫放視窗的繪製和處理
void drawBufferWindow(HDC hdc, RECT rc, GlobalState& state) {
    HBRUSH hBg = CreateSolidBrush(state.bufferBackgroundColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    
    HPEN hBorderPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));
    HPEN hOldBorderPen = (HPEN)SelectObject(hdc, hBorderPen);
    HBRUSH hOldBorderBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, rc.right, rc.bottom);
    SelectObject(hdc, hOldBorderBrush);
    SelectObject(hdc, hOldBorderPen);
    DeleteObject(hBorderPen);
    
    RECT textArea = {10, 10, rc.right - 10, rc.bottom - CONTROL_BAR_HEIGHT - 2};
    
    HFONT hFont = CreateFontW(state.bufferFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, state.bufferFontName.c_str());
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    SetTextColor(hdc, state.bufferTextColor);
    SetBkMode(hdc, TRANSPARENT);
    
    if (!state.bufferText.empty()) {
		drawTextWithSelection(hdc, textArea, state);
	}
    
   // 繪製游標（只在沒有選取時顯示）
	if (state.bufferHasFocus && state.bufferShowCursor && !state.hasSelection) {
    // 使用新的座標轉換函數
    POINT cursorPos = BufferManager::getPointFromTextPosition(state, state.bufferCursorPos); 
        
        HPEN hCursorPen = CreatePen(PS_SOLID, 1, state.bufferCursorColor);
        HPEN hOldCursorPen = (HPEN)SelectObject(hdc, hCursorPen);
        MoveToEx(hdc, cursorPos.x, cursorPos.y, NULL);  // ✅ 正确
         LineTo(hdc, cursorPos.x, cursorPos.y + state.bufferFontSize);  // ✅ 正确
        SelectObject(hdc, hOldCursorPen);
        DeleteObject(hCursorPen);
    }
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
    
    int controlY = rc.bottom - CONTROL_BAR_HEIGHT;
    RECT controlRect = {0, controlY, rc.right, rc.bottom};
    HBRUSH hControlBg = CreateSolidBrush(RGB(245, 245, 245));
    FillRect(hdc, &controlRect, hControlBg);
    DeleteObject(hControlBg);
    
    std::wstring statsText = L"字數: " + std::to_wstring(state.bufferText.length()) + L" | 位置: " + std::to_wstring(state.bufferCursorPos);
    SetTextColor(hdc, RGB(100, 100, 100));
    TextOutW(hdc, 10, controlY + 2, statsText.c_str(), statsText.length());
    
    // 按鈕放在狀態文字下方
    int buttonY = controlY + 20;              // 狀態文字下方
    int buttonHeight = 20;                    // 按鈕高度
    int buttonWidth = 70;                     // 適中的按鈕寬度
    int buttonSpacing = 8;                    // 按鈕間距
    int leftMargin = 15;                      // 左邊距，與狀態文字對齊
    
    // 確保按鈕不會超出控制列底部
    if (buttonY + buttonHeight > rc.bottom - 3) {
        buttonY = rc.bottom - buttonHeight - 3;
    }
    
    // 從左往右排列按鈕（更符合閱讀習慣）
    int saveButtonX = leftMargin;
    int clearButtonX = saveButtonX + buttonWidth + buttonSpacing;
    int sendButtonX = clearButtonX + buttonWidth + buttonSpacing;
    
    // 檢查最右邊按鈕是否超出視窗，如果超出就縮小按鈕
    int totalButtonWidth = sendButtonX + buttonWidth;
    if (totalButtonWidth > rc.right - 10) {
        // 重新計算更緊湊的布局
        buttonWidth = 50;
        buttonSpacing = 5;
        saveButtonX = leftMargin;
        clearButtonX = saveButtonX + buttonWidth + buttonSpacing;
        sendButtonX = clearButtonX + buttonWidth + buttonSpacing;
    }
    
    // === 儲存按鈕 ===
    state.saveButtonRect = {saveButtonX, buttonY, saveButtonX + buttonWidth, buttonY + buttonHeight};
    COLORREF newSaveColor = state.saveButtonHover ? RGB(100, 150, 255) : RGB(220, 220, 220);
    HBRUSH hNewSaveBrush = CreateSolidBrush(newSaveColor);
    FillRect(hdc, &state.saveButtonRect, hNewSaveBrush);
    DeleteObject(hNewSaveBrush);
    
    HPEN hNewSavePen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
    HPEN hNewSaveOldPen = (HPEN)SelectObject(hdc, hNewSavePen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, state.saveButtonRect.left, state.saveButtonRect.top, 
              state.saveButtonRect.right, state.saveButtonRect.bottom);
    SelectObject(hdc, hNewSaveOldPen);
    DeleteObject(hNewSavePen);
    
    SetTextColor(hdc, RGB(80, 80, 80));
    DrawTextW(hdc, L"儲存", -1, &state.saveButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // === 清空按鈕 ===
    state.clearButtonRect = {clearButtonX, buttonY, clearButtonX + buttonWidth, buttonY + buttonHeight};
    COLORREF newClearColor = state.clearButtonHover ? RGB(255, 100, 100) : RGB(220, 220, 220);
    HBRUSH hNewClearBrush = CreateSolidBrush(newClearColor);
    FillRect(hdc, &state.clearButtonRect, hNewClearBrush);
    DeleteObject(hNewClearBrush);
    
    HPEN hNewClearPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
    HPEN hNewClearOldPen = (HPEN)SelectObject(hdc, hNewClearPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, state.clearButtonRect.left, state.clearButtonRect.top, 
              state.clearButtonRect.right, state.clearButtonRect.bottom);
    SelectObject(hdc, hNewClearOldPen);
    DeleteObject(hNewClearPen);
    
    DrawTextW(hdc, L"清空", -1, &state.clearButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // === 發送按鈕 ===
    state.sendButtonRect = {sendButtonX, buttonY, sendButtonX + buttonWidth, buttonY + buttonHeight};
    COLORREF newSendColor = state.sendButtonHover ? RGB(100, 180, 100) : RGB(220, 220, 220);
    HBRUSH hNewSendBrush = CreateSolidBrush(newSendColor);
    FillRect(hdc, &state.sendButtonRect, hNewSendBrush);
    DeleteObject(hNewSendBrush);
    
    HPEN hNewSendPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
    HPEN hNewSendOldPen = (HPEN)SelectObject(hdc, hNewSendPen);
    SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, state.sendButtonRect.left, state.sendButtonRect.top, 
              state.sendButtonRect.right, state.sendButtonRect.bottom);
    SelectObject(hdc, hNewSendOldPen);
    DeleteObject(hNewSendPen);
    
    DrawTextW(hdc, L"Enter發送", -1, &state.sendButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

LRESULT CALLBACK BufferProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
		 case WM_ERASEBKGND:
            return 1;
          case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            // 双缓冲绘制
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
            
            drawBufferWindow(memDC, rc, g_state);
            
            BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
            
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
    int x = LOWORD(lp); 
    int y = HIWORD(lp);
    
    // 檢查按鈕點擊（優先處理）
    if (isPointInSendButton(x, y, g_state)) {
        BufferManager::sendBufferContent(g_state);
        return 0;
    }
    
    if (isPointInClearButton(x, y, g_state)) {
        BufferManager::clearBufferWithConfirm(g_state);
        return 0;
    }
    
    if (isPointInSaveButton(x, y, g_state)) {
        BufferManager::saveBufferToTimestampedFile(g_state);
        return 0;
    }
    
    // 清除之前的選取
    BufferManager::clearSelection(g_state);
    
    // 開始新的選取或設定游標
    SetCapture(hwnd);
    g_state.bufferHasFocus = true;
    SetTimer(hwnd, 1, 500, NULL);
    g_state.bufferShowCursor = true;
    
    BufferManager::startSelection(g_state, x, y);
    BufferManager::setCursorPosition(g_state, x, y);
    return 0;
}
        
        case WM_MOUSEMOVE: {
    int x = LOWORD(lp); 
    int y = HIWORD(lp);
    
    bool wasSendHover = g_state.sendButtonHover;
    bool wasClearHover = g_state.clearButtonHover;
    bool wasSaveHover = g_state.saveButtonHover;
    
    g_state.sendButtonHover = isPointInSendButton(x, y, g_state);
    g_state.clearButtonHover = isPointInClearButton(x, y, g_state);
    g_state.saveButtonHover = isPointInSaveButton(x, y, g_state);
    
    
    if (g_state.isSelecting) {
        BufferManager::updateSelection(g_state, x, y);
    }
    
    
    if (wasSendHover != g_state.sendButtonHover || 
        wasClearHover != g_state.clearButtonHover || 
        wasSaveHover != g_state.saveButtonHover) {
        InvalidateRect(hwnd, nullptr, TRUE);
    }
    return 0;
}

case WM_LBUTTONUP: {
    if (GetCapture() == hwnd) {
        ReleaseCapture();
        BufferManager::endSelection(g_state);
    }
    return 0;
}

case WM_RBUTTONDOWN: {
    int x = LOWORD(lp);
    int y = HIWORD(lp);
    
    // 創建右鍵選單
    HMENU hMenu = CreatePopupMenu();
    
    if (g_state.hasSelection) {
        AppendMenu(hMenu, MF_STRING, 1001, L"複製 (Ctrl+C)");
        AppendMenu(hMenu, MF_STRING, 1002, L"剪下 (Ctrl+X)");
        AppendMenu(hMenu, MF_STRING, 1003, L"刪除");
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    }
    
    AppendMenu(hMenu, MF_STRING, 1004, L"全選 (Ctrl+A)");
    
    if (!g_state.bufferText.empty()) {
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        AppendMenu(hMenu, MF_STRING, 1005, L"清空全部");
    }
    
    POINT pt = {x, y};
    ClientToScreen(hwnd, &pt);
    
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                            pt.x, pt.y, 0, hwnd, NULL);
    
    DestroyMenu(hMenu);
    
    // 處理選單命令
    switch (cmd) {
        case 1001: BufferManager::copySelection(g_state); break;
        case 1002: BufferManager::cutSelection(g_state); break;
        case 1003: BufferManager::deleteSelection(g_state); break;
        case 1004: BufferManager::selectAll(g_state); break;
        case 1005: BufferManager::clearBufferWithConfirm(g_state); break;
    }
    
    return 0;
}
    

case WM_KEYDOWN: {
    bool ctrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    
    switch (wp) {
        case 'A':
            if (ctrlPressed) {
                BufferManager::selectAll(g_state);
                return 0;
            }
            break;
            
        case 'C':
            if (ctrlPressed && g_state.hasSelection) {
                BufferManager::copySelection(g_state);
                return 0;
            }
            break;
            
        case 'X':
            if (ctrlPressed && g_state.hasSelection) {
                BufferManager::cutSelection(g_state);
                return 0;
            }
            break;
            
        case 'V':
            if (ctrlPressed) {
                // 如果有選取文字，先刪除
                if (g_state.hasSelection) {
                    BufferManager::deleteSelection(g_state);
                }
                
                // 從剪貼簿貼上
                if (OpenClipboard(hwnd)) {
                    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
                    if (hData) {
                        wchar_t* pszText = (wchar_t*)GlobalLock(hData);
                        if (pszText) {
                            std::wstring pastedText(pszText);
                            BufferManager::insertTextAtCursor(g_state, pastedText);
                            Utils::updateStatus(g_state, L"已貼上 " + std::to_wstring(pastedText.length()) + L" 個字符");
                            GlobalUnlock(hData);
                        }
                    }
                    CloseClipboard();
                }
                return 0;
            }
            break;
            
        case VK_DELETE:
            if (g_state.hasSelection) {
                BufferManager::deleteSelection(g_state);
                return 0;
            }
            break;
            
        case VK_ESCAPE:
            if (g_state.hasSelection) {
                BufferManager::clearSelection(g_state);
                return 0;
            }
            break;
    }
    break;
}    
        case WM_TIMER: {
            if (wp == 1) {
                g_state.bufferShowCursor = !g_state.bufferShowCursor;
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            return 0;
        }
        
        case WM_SETFOCUS: {
            g_state.bufferHasFocus = true;
            SetTimer(hwnd, 1, 500, NULL);
            g_state.bufferShowCursor = true;
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
        
        case WM_KILLFOCUS: {
            g_state.bufferHasFocus = false;
            KillTimer(hwnd, 1);
            InvalidateRect(hwnd, nullptr, TRUE);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, msg, wp, lp);
}

// OptimizedUI工具列繪製函數
void drawOptimizedToolbar(HDC hdc, RECT rc, GlobalState& state) {
    // 背景
    HBRUSH hBg = CreateSolidBrush(state.uiColors.toolbarBgColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    
    // 邊框
    HPEN hPen = CreatePen(PS_SOLID, 1, state.uiColors.toolbarBorderColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, rc.right, rc.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    SetBkMode(hdc, TRANSPARENT);
    
    int x = 5;
    int y = (rc.bottom - BUTTON_HEIGHT) / 2;
    
    // 筆劃標識
    HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft JhengHei");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    SetTextColor(hdc, RGB(60, 60, 60));
    TextOutW(hdc, x, y + 3, L"筆劃", 2);
    x += 50;
    
    // 模式指示器
    state.toolbarElements.modeIndicatorRect = {x, y, x + MODE_BUTTON_WIDTH, y + BUTTON_HEIGHT};
    COLORREF modeColor = state.chineseMode ? state.uiColors.modeActiveColor : state.uiColors.modeInactiveColor;
    if (state.toolbarElements.modeIndicatorHover) modeColor = state.uiColors.buttonHoverColor;
    
    HBRUSH hModeBrush = CreateSolidBrush(modeColor);
    FillRect(hdc, &state.toolbarElements.modeIndicatorRect, hModeBrush);
    DeleteObject(hModeBrush);
    
    SetTextColor(hdc, RGB(255, 255, 255));
    std::wstring modeText = state.chineseMode ? L"中" : L"EN";
    RECT modeTextRect = state.toolbarElements.modeIndicatorRect;
    DrawTextW(hdc, modeText.c_str(), -1, &modeTextRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    x += 40;
    
    // 狀態指示器
    state.toolbarElements.statusIndicatorRect = {x, y + 8, x + 6, y + 14};
    COLORREF statusColor = state.uiColors.statusReadyColor;
    if (state.inputError) statusColor = state.uiColors.statusErrorColor;
    else if (state.isInputting) statusColor = state.uiColors.statusInputColor;
    else if (state.bufferMode) statusColor = state.uiColors.statusBufferColor;
    
    HBRUSH hStatusBrush = CreateSolidBrush(statusColor);
    HPEN hStatusPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
    SelectObject(hdc, hStatusPen);
    SelectObject(hdc, hStatusBrush);
    Ellipse(hdc, state.toolbarElements.statusIndicatorRect.left, state.toolbarElements.statusIndicatorRect.top,
            state.toolbarElements.statusIndicatorRect.right, state.toolbarElements.statusIndicatorRect.bottom);
    DeleteObject(hStatusBrush);
    DeleteObject(hStatusPen);
    x += 12;
    
    // 選單按鈕
    state.toolbarElements.menuButtonRect = {x, y, x + 40, y + BUTTON_HEIGHT};
    if (state.toolbarElements.menuButtonHover) {
        HBRUSH hMenuBrush = CreateSolidBrush(state.uiColors.buttonHoverColor);
        FillRect(hdc, &state.toolbarElements.menuButtonRect, hMenuBrush);
        DeleteObject(hMenuBrush);
    }
    SetTextColor(hdc, RGB(60, 60, 60));
    DrawTextW(hdc, L"☰", -1, &state.toolbarElements.menuButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    x += 45;
    
    // 暫放模式按鈕
    state.toolbarElements.bufferButtonRect = {x, y, x + SMALL_BUTTON_WIDTH, y + BUTTON_HEIGHT};
    COLORREF bufferColor = state.bufferMode ? state.uiColors.bufferButtonActiveColor : state.uiColors.bufferButtonInactiveColor;
    if (state.toolbarElements.bufferButtonHover) bufferColor = state.uiColors.buttonHoverColor;
    
    HBRUSH hBufferBrush = CreateSolidBrush(bufferColor);
    FillRect(hdc, &state.toolbarElements.bufferButtonRect, hBufferBrush);
    DeleteObject(hBufferBrush);

    HFONT hBufferFont = CreateFontW(25, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft JhengHei");
    HFONT hOldBufferFont = (HFONT)SelectObject(hdc, hBufferFont);
    SetTextColor(hdc, RGB(60, 60, 60));
    DrawTextW(hdc, L"⌘", -1, &state.toolbarElements.bufferButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldBufferFont);
    DeleteObject(hBufferFont);
    x += 40;
    
    // 恢復跟隨鼠標位置按鈕
    state.toolbarElements.restoreButtonRect = {x, y, x + SMALL_BUTTON_WIDTH, y + BUTTON_HEIGHT};
    if (state.toolbarElements.restoreButtonHover) {
        HBRUSH hResetBrush = CreateSolidBrush(state.uiColors.buttonHoverColor);
        FillRect(hdc, &state.toolbarElements.restoreButtonRect, hResetBrush);
        DeleteObject(hResetBrush);
    }

    HFONT hResetFont = CreateFontW(22, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft JhengHei");
    HFONT hOldResetFont = (HFONT)SelectObject(hdc, hResetFont);
    SetTextColor(hdc, RGB(60, 60, 60));
    DrawTextW(hdc, L"⿻", -1, &state.toolbarElements.restoreButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, hOldResetFont);
    DeleteObject(hResetFont);
    x += 40;
    
    // 最小化按鈕
    state.toolbarElements.minimizeButtonRect = {x, y, x + 20, y + BUTTON_HEIGHT};
    if (state.toolbarElements.minimizeButtonHover) {
        HBRUSH hMinBrush = CreateSolidBrush(state.uiColors.buttonHoverColor);
        FillRect(hdc, &state.toolbarElements.minimizeButtonRect, hMinBrush);
        DeleteObject(hMinBrush);
    }
    SetTextColor(hdc, RGB(60, 60, 60));
    DrawTextW(hdc, L"－", -1, &state.toolbarElements.minimizeButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    x += 25;
    
    // 關閉按鈕
    state.toolbarElements.closeButtonRect = {x, y, x + 20, y + BUTTON_HEIGHT};
    COLORREF closeColor = state.toolbarElements.closeButtonHover ? RGB(255, 70, 70) : state.uiColors.closeButtonColor;
    HBRUSH hCloseBrush = CreateSolidBrush(closeColor);
    FillRect(hdc, &state.toolbarElements.closeButtonRect, hCloseBrush);
    DeleteObject(hCloseBrush);
    
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextW(hdc, L"×", -1, &state.toolbarElements.closeButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

// OptimizedUI按鈕點擊檢測
bool isPointInOptimizedButton(int x, int y, const RECT& buttonRect) {
    return Utils::isPointInRect(x, y, buttonRect);
}

// OptimizedUI按鈕懸停狀態更新
void updateOptimizedButtonHover(int x, int y, GlobalState& state) {
    bool newModeHover = isPointInOptimizedButton(x, y, state.toolbarElements.modeIndicatorRect);
    bool newMenuHover = isPointInOptimizedButton(x, y, state.toolbarElements.menuButtonRect);
    bool newBufferHover = isPointInOptimizedButton(x, y, state.toolbarElements.bufferButtonRect);
    bool newRestoreHover = isPointInOptimizedButton(x, y, state.toolbarElements.restoreButtonRect);
    bool newMinimizeHover = isPointInOptimizedButton(x, y, state.toolbarElements.minimizeButtonRect);
    bool newCloseHover = isPointInOptimizedButton(x, y, state.toolbarElements.closeButtonRect);
    
    bool needRedraw = false;
    if (newModeHover != state.toolbarElements.modeIndicatorHover || 
        newMenuHover != state.toolbarElements.menuButtonHover ||
        newBufferHover != state.toolbarElements.bufferButtonHover ||
        newRestoreHover != state.toolbarElements.restoreButtonHover || 
        newMinimizeHover != state.toolbarElements.minimizeButtonHover || 
        newCloseHover != state.toolbarElements.closeButtonHover) {
        needRedraw = true;
    }
    
    state.toolbarElements.modeIndicatorHover = newModeHover;
    state.toolbarElements.menuButtonHover = newMenuHover;
    state.toolbarElements.bufferButtonHover = newBufferHover;
    state.toolbarElements.restoreButtonHover = newRestoreHover;
    state.toolbarElements.minimizeButtonHover = newMinimizeHover;
    state.toolbarElements.closeButtonHover = newCloseHover;
    
    if (needRedraw && state.hWnd) {
        InvalidateRect(state.hWnd, nullptr, TRUE);
    }
}

// 暫放視窗跟隨工具列移動
void updateBufferWindowPosition(GlobalState& state) {
    if (!state.bufferMode || !state.hBufferWnd) {
        return;
    }
    
    // 獲得工具列位置
    RECT toolbarRect;
    GetWindowRect(state.hWnd, &toolbarRect);
    
    // 計算暫放視窗應該的位置（工具列正下方）
    int bufferX = toolbarRect.left;
    int bufferY = toolbarRect.bottom + 5;
    
    // 計算暫放視窗高度
    int windowHeight = BufferManager::calculateBufferWindowHeight(state);
    
    // 確保暫放視窗可見並正確定位
    SetWindowPos(state.hBufferWnd, HWND_TOPMOST, 
                bufferX, bufferY, FIXED_WIDTH, windowHeight,
                SWP_NOACTIVATE | SWP_SHOWWINDOW);
    
    InvalidateRect(state.hBufferWnd, nullptr, TRUE);
}

// OptimizedUI工具列拖拽處理
void handleOptimizedToolbarDrag(HWND hwnd, POINT currentPos, GlobalState& state) {
    // 計算新位置
    int newX = currentPos.x - state.dragState.dragOffset.x;
    int newY = currentPos.y - state.dragState.dragOffset.y;
    
    // 移動工具列
    SetWindowPos(hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
    
    // 更新全域位置記錄
    PositionManager::g_toolbarPos.x = newX;
    PositionManager::g_toolbarPos.y = newY;
    
    // 暫放視窗跟隨移動
    updateBufferWindowPosition(state);
}

// OptimizedUI候選字視窗拖拽處理
void handleOptimizedCandidateDrag(HWND hwnd, POINT currentPos, GlobalState& state) {
    int newX = currentPos.x - state.dragState.dragOffset.x;
    int newY = currentPos.y - state.dragState.dragOffset.y;
    
    // 移動候選字視窗
    RECT candRect;
    GetWindowRect(hwnd, &candRect);
    int candWidth = candRect.right - candRect.left;
    int candHeight = candRect.bottom - candRect.top;
    
    SetWindowPos(hwnd, NULL, newX, newY, candWidth, candHeight, SWP_NOZORDER | SWP_SHOWWINDOW);
}

// OptimizedUI視窗類別註冊
bool registerOptimizedWindowClasses(HINSTANCE hInstance) {
    // 註冊主視窗類別
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = OptimizedWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"IME_MAIN_OPTIMIZED";
    wc.hbrBackground = CreateSolidBrush(RGB(240,240,240));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc)) {
        return false;
    }
    
    // 註冊候選視窗類別
    WNDCLASSW wc2 = {0};
    wc2.lpfnWndProc = OptimizedCandProc;
    wc2.hInstance = hInstance;
    wc2.lpszClassName = L"IME_CAND_OPTIMIZED";
    wc2.hbrBackground = NULL;
    wc2.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc2)) {
        return false;
    }
    
    // 暫放視窗類別使用現有的
    return registerWindowClasses(hInstance);
}

// 創建字碼輸入視窗
bool createInputWindow(HINSTANCE hInstance, GlobalState& state) {
    // 註冊字碼視窗類別
    WNDCLASSW wc = {0};
    wc.lpfnWndProc = InputProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"IME_INPUT";
    wc.hbrBackground = CreateSolidBrush(state.inputBackgroundColor); // 使用配置的背景色
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc)) {
        return false;
    }
    
    // 創建字碼視窗，使用配置的尺寸
    state.hInputWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"IME_INPUT",
        L"",
        WS_POPUP | WS_BORDER,
        100, 150, 
        state.inputWindowWidth,   // 使用配置的寬度
        state.inputWindowHeight,  // 使用配置的高度
        NULL, NULL, hInstance, NULL
    );
    
    return state.hInputWnd != NULL;
}

// 修復：字碼輸入視窗繪製 - 加入3+3提示
void drawInputWindow(HDC hdc, RECT rc, const GlobalState& state) {
    // 使用配置的背景色（替代寫死的白色）
    HBRUSH hBg = CreateSolidBrush(state.inputBackgroundColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    
    // 使用配置的邊框色（替代寫死的灰色）
    HPEN hPen = CreatePen(PS_SOLID, 1, state.inputBorderColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, rc.right, rc.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    // 使用配置的字型大小和名稱（替代寫死的16和Microsoft JhengHei）
    HFONT hFont = CreateFontW(state.inputFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, state.inputFontName.c_str());
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    SetBkMode(hdc, TRANSPARENT);
    
    // 使用 getInputDisplay 來獲取包含3+3提示的顯示文字
    std::wstring displayText;
    if (state.input.empty()) {
        displayText = state.chineseMode ? L"請輸入筆劃代碼 (u i o j k l)" : L"English Input Mode";
        SetTextColor(hdc, state.inputHintTextColor); // 使用配置的提示文字顏色
    } else {
        displayText = Dictionary::getInputDisplay(state);
        
        if (state.inputError) {
            SetTextColor(hdc, state.inputErrorTextColor); // 使用配置的錯誤文字顏色
        } else {
            SetTextColor(hdc, state.inputTextColor); // 使用配置的正常文字顏色
        }
    }
    
    // 處理過長文字的顯示
    int maxWidth = rc.right - 20;
    SIZE textSize;
    GetTextExtentPoint32W(hdc, displayText.c_str(), displayText.length(), &textSize);
    
    if (textSize.cx > maxWidth) {
        // 文字過長時縮小字體
        DeleteObject(hFont);
        int smallerSize = (maxWidth * state.inputFontSize) / textSize.cx;
        if (smallerSize < 10) smallerSize = 10;
        
        hFont = CreateFontW(smallerSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, state.inputFontName.c_str());
        SelectObject(hdc, hFont);
    }
    
    TextOutW(hdc, 10, 6, displayText.c_str(), displayText.length());
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}

// OptimizedUI視窗建立
bool createOptimizedWindows(HINSTANCE hInstance, GlobalState& state) {
    // 創建主工具列視窗
    state.hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
        L"IME_MAIN_OPTIMIZED", 
        L"中文筆劃輸入法",
        WS_POPUP | WS_BORDER, 
        PositionManager::g_toolbarPos.x, 
        PositionManager::g_toolbarPos.y, 
        TOOLBAR_WIDTH, 
        TOOLBAR_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (!state.hWnd) return false;
    
    // 創建候選視窗
    state.hCandWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
        L"IME_CAND_OPTIMIZED", 
        L"",
        WS_POPUP | WS_BORDER, 
        100, 180, state.candidateWidth, state.candidateHeight,
        NULL, NULL, hInstance, NULL
    );
    
    if (!state.hCandWnd) return false;
    
    // 創建字碼輸入視窗
    if (!createInputWindow(hInstance, state)) {
        return false;
    }
    
    // 創建暫放視窗（使用既有函數）
    state.hBufferWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW, 
        L"IME_BUFFER", 
        L"",
        WS_POPUP | WS_BORDER, 
        100, 280, FIXED_WIDTH, MIN_HEIGHT,
        NULL, NULL, hInstance, NULL
    );
    
    if (!state.hBufferWnd) return false;
    
    return true;
}

// UI模式切換
void switchToOptimizedUI(GlobalState& state) {
    state.useOptimizedUI = true;
    if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
}

void switchToClassicUI(GlobalState& state) {
    state.useOptimizedUI = false;
    if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
}

// OptimizedUI主視窗程序
LRESULT CALLBACK OptimizedWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
		case WM_DESTROY:
    // 清理資源
    if (g_hKeyboardHook) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }
    
    // 銷毀所有視窗 - 新增這部分
    if (g_state.hInputWnd) {
        DestroyWindow(g_state.hInputWnd);
        g_state.hInputWnd = NULL;
    }
    if (g_state.hCandWnd) {
        DestroyWindow(g_state.hCandWnd);
        g_state.hCandWnd = NULL;
    }
    if (g_state.hBufferWnd) {
        DestroyWindow(g_state.hBufferWnd);
        g_state.hBufferWnd = NULL;
    }
	
    // 定時器清理
    KillTimer(hwnd, 999);	
    
    Dictionary::saveUserDict(g_state);
    PositionManager::savePositions(g_state);
    TrayManager::removeTrayIcon(&g_trayIcon);
    
    PostQuitMessage(0);
    return 0;
	
        case WM_CREATE: {
            // 使用配置檔案中的間隔設定
            if (g_state.forceStayOnTop) {
                SetTimer(hwnd, 999, g_state.topmostCheckInterval, NULL);
            }
            return 0;
        }	
        
		case WM_TIMER: {
			if (wp == 997) {
				// 新增：延遲處理螢幕模式變更
				KillTimer(hwnd, 997);
        
				ScreenManager::handleDisplayChange();
				PositionManager::adjustPositionForScreenMode(g_state);
        
				// 確保視窗可見
				if (!IsWindowVisible(hwnd)) {
					ShowWindow(hwnd, SW_SHOW);
				}
        
				// 強制重繪
				InvalidateRect(hwnd, nullptr, TRUE);
				UpdateWindow(hwnd);
        
				return 0;
			}
			else if (wp == 998) {
				// 新增：重試定位
				KillTimer(hwnd, 998);
				PositionManager::adjustPositionForScreenMode(g_state);
				return 0;
			}
			else if (wp == 996) {
				// 新增：初始化位置檢查
				KillTimer(hwnd, 996);
        
				// 確保視窗在可見位置
				if (!PositionManager::isPositionVisible(g_state)) {
					PositionManager::forceResetToSafePosition(g_state);
				}
        
				// 確保視窗顯示
				ShowWindow(g_state.hWnd, SW_SHOW);
				SetForegroundWindow(g_state.hWnd);
        
				return 0;
			}
			else if (wp == 999) {
            
                // 強制保持前置
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, 
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                
                // 同時確保其他關鍵視窗也保持前置
                if (g_state.hCandWnd && IsWindowVisible(g_state.hCandWnd)) {
                    SetWindowPos(g_state.hCandWnd, HWND_TOPMOST, 0, 0, 0, 0,
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                }
                if (g_state.hInputWnd && IsWindowVisible(g_state.hInputWnd)) {
                    SetWindowPos(g_state.hInputWnd, HWND_TOPMOST, 0, 0, 0, 0,
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                }
                if (g_state.hBufferWnd && IsWindowVisible(g_state.hBufferWnd)) {
                    SetWindowPos(g_state.hBufferWnd, HWND_TOPMOST, 0, 0, 0, 0,
                                SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
                }
            }
            return 0;
        }
		
        case WM_DISPLAYCHANGE: {
            // 延遲處理，等待系統完成切換
            static UINT_PTR delayTimerId = 0;
            if (delayTimerId) {
                KillTimer(hwnd, delayTimerId);
            }
            delayTimerId = SetTimer(hwnd, 997, 500, NULL);
            return 0;
        }

        case WM_USER+100: {
            DWORD key = (DWORD)wp;
            if (g_state.chineseMode) {
                if (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L' || key == 'P' ||
                    key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0) {
                    InputHandler::processStroke(g_state, key);
                    return 0;
                }
                if (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 ||
                    key == VK_OEM_1 || key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 ||
                    key == VK_SPACE || key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||
                    (key == '1' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '2' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (key == '3' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '4' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (key == '5' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '6' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (key == '7' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '8' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                    (key == '9' && (GetKeyState(VK_SHIFT) & 0x8000)) || (key == '0' && (GetKeyState(VK_SHIFT) & 0x8000))) {
                    InputHandler::processPunctuator(g_state, key);
                    return 0;
                }
            }
            
            // 功能鍵處理
            if (key == VK_DOWN) { Dictionary::changePage(g_state, 1); return 0; }
            if (key == VK_UP) { Dictionary::changePage(g_state, -1); return 0; }
            if (key >= '1' && key <= '9') { Dictionary::selectCandidate(g_state, key - '1'); return 0; }
            if (key == VK_BACK) {
                if (!g_state.input.empty()) {
                    g_state.input.pop_back();
                    Dictionary::updateCandidates(g_state);
                    InvalidateRect(hwnd, nullptr, TRUE);
                }
                return 0;
            }
            if (key == VK_SPACE) { Dictionary::selectCandidate(g_state, 0); return 0; }
            if (key == VK_RETURN) { InputHandler::handleEnterKeySmartly(g_state); return 0; }
            if (key == VK_ESCAPE) {
                g_state.input.clear();
                g_state.candidates.clear();
                g_state.candidateCodes.clear();
                g_state.showCand = false;
                g_state.isInputting = false;
                g_state.inputError = false;
                g_state.showPunctMenu = false;
                if (g_state.hCandWnd) ShowWindow(g_state.hCandWnd, SW_HIDE);
                if (g_state.hInputWnd) ShowWindow(g_state.hInputWnd, SW_HIDE);
                Utils::updateStatus(g_state, L"輸入已取消");
                InvalidateRect(hwnd, nullptr, TRUE);
                return 0;
            }
            break;
        }
		
		case WM_USER+200:  // 在 OptimizedWndProc 中添加這個
			TrayManager::processTrayMessage(hwnd, lp, g_state);
			return 0;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            
            // 使用OptimizedUI繪製
            drawOptimizedToolbar(hdc, rc, g_state);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
            
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lp); 
            int y = HIWORD(lp);
            
            // OptimizedUI按鈕點擊檢測
            if (isPointInOptimizedButton(x, y, g_state.toolbarElements.closeButtonRect)) {
                if (MessageBoxW(hwnd, L"確定要關閉輸入法嗎？", L"確認關閉", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                }
                return 0;
            }
            
            if (isPointInOptimizedButton(x, y, g_state.toolbarElements.modeIndicatorRect)) {
                InputHandler::toggleInputMode(g_state);
                return 0;
            }
            
            if (isPointInOptimizedButton(x, y, g_state.toolbarElements.menuButtonRect)) {
                HMENU hMenu = CreatePopupMenu();
                AppendMenu(hMenu, MF_STRING, 1001, L"標點符號選單");
                AppendMenu(hMenu, MF_STRING, 1002, L"重新載入字典");
                AppendMenu(hMenu, MF_STRING, 1005, L"重新載入配置");
                AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
                AppendMenu(hMenu, MF_STRING, 1003, L"關於");
                
                POINT pt;
                GetCursorPos(&pt);
                TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(hMenu);
                return 0;
            }
            
            // ⌘暫放按鈕點擊處理
            if (isPointInOptimizedButton(x, y, g_state.toolbarElements.bufferButtonRect)) {
                BufferManager::toggleBufferMode(g_state);
                InvalidateRect(hwnd, nullptr, TRUE);  // 重繪工具列以更新按鈕狀態
                return 0;
            }
            
            if (isPointInOptimizedButton(x, y, g_state.toolbarElements.restoreButtonRect)) {
                PositionManager::g_useUserPosition = false;
                PositionManager::savePositions(g_state);
                Utils::updateStatus(g_state, L"已恢復滑鼠跟隨模式");
                return 0;
            }
            
            if (isPointInOptimizedButton(x, y, g_state.toolbarElements.minimizeButtonRect)) {
                TrayManager::hideToTray(hwnd, &g_trayIcon);
                return 0;
            }
            
            // 開始拖曳工具列
            g_state.dragState.isToolbarDragging = true;
            SetCapture(hwnd);
            
            POINT pt;
            GetCursorPos(&pt);
            RECT toolbarRect;
            GetWindowRect(hwnd, &toolbarRect);
            g_state.dragState.dragOffset.x = pt.x - toolbarRect.left;
            g_state.dragState.dragOffset.y = pt.y - toolbarRect.top;
            
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (g_state.dragState.isToolbarDragging) {
                POINT pt;
                GetCursorPos(&pt);
                handleOptimizedToolbarDrag(hwnd, pt, g_state);
                return 0;
            }
            else {
                int x = LOWORD(lp); 
                int y = HIWORD(lp);
                updateOptimizedButtonHover(x, y, g_state);
            }
            return 0;
        }

        case WM_LBUTTONUP: { 
            if (g_state.dragState.isToolbarDragging) { 
                g_state.dragState.isToolbarDragging = false; 
                ReleaseCapture(); 
                PositionManager::savePositions(g_state);
                return 0; 
            } 
            break; 
        }
		
		case WM_USER + 301: { // 新增：自定義螢幕模式變更消息
			//bool isExtended = (wp == 1);
    
			// 立即處理模式變更
			PositionManager::adjustPositionForScreenMode(g_state);
    
			// 確保視窗在正確位置
			SetWindowPos(hwnd, HWND_TOPMOST,
				PositionManager::g_toolbarPos.x,
				PositionManager::g_toolbarPos.y,
				0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
    
			return 0;
		}
            
        case WM_COMMAND: {
            // 使用與原WndProc相同的命令處理邏輯
            switch (LOWORD(wp)) {
                case 1001: InputHandler::showPunctMenu(g_state); break;
                case 1002: 
                    Dictionary::loadMainDict("Zi-Ma-Biao.txt", g_state);
                    Utils::updateStatus(g_state, L"字典已重新載入");
                    break;
                case 1003:
					showAboutDialog(hwnd, true);
					break;
                case 1005:
                    ConfigLoader::refreshConfigs(g_state);
                    MessageBoxW(hwnd, L"配置已重新載入！", L"提示", MB_OK | MB_ICONINFORMATION);
                    break;
                    
                // 托盤選單命令處理
                case 2001: TrayManager::showFromTray(hwnd, &g_trayIcon); break;
                case 2002: 
                    PositionManager::g_useUserPosition = false;
                    PositionManager::savePositions(g_state);
                    Utils::updateStatus(g_state, L"已重置為滑鼠跟隨模式");
                    break;
                case 2003: PostMessage(hwnd, WM_CLOSE, 0, 0); break;
                case 2004: InputHandler::toggleInputMode(g_state); break;
                case 2005: InputHandler::showPunctMenu(g_state); break;
                case 2006: ConfigLoader::refreshConfigs(g_state); break;
                case 2007: 
                    PositionManager::g_useUserPosition = false;
                    PositionManager::savePositions(g_state);
                    Utils::updateStatus(g_state, L"已取消固定位置");
                    break;
                case 2008: 
                    showAboutDialog(hwnd, g_state.useOptimizedUI);
                    break;
                case 2009: // 重啟輸入法
				    if (MessageBoxW(hwnd, L"確定要重啟輸入法嗎？\n\n重啟後將保留所有設定和學習記錄。", 
				        L"確認重啟", MB_YESNO | MB_ICONQUESTION) == IDYES) {
				        // 直接實現重啟功能
				        wchar_t exePath[MAX_PATH];
				        GetModuleFileNameW(NULL, exePath, MAX_PATH);
                        
				        STARTUPINFOW si = {0};
				        PROCESS_INFORMATION pi = {0};
				        si.cb = sizeof(STARTUPINFOW);
                        
				        if (CreateProcessW(exePath, NULL, NULL, NULL, FALSE, 
				                          0, NULL, NULL, &si, &pi)) {
				            CloseHandle(pi.hProcess);
				            CloseHandle(pi.hThread);
                            
				            Sleep(500);
                            
				            PostQuitMessage(0);
				        } else {
				            MessageBoxW(NULL, L"重啟輸入法失敗", L"錯誤", MB_OK | MB_ICONERROR);
				        }
				    }
				    break;
                case 2010: BufferManager::sendBufferContent(g_state); break;
                case 2011: BufferManager::clearBufferWithConfirm(g_state); break;
                case 2012: BufferManager::toggleBufferMode(g_state); break;
            }
            return 0;
        }
        
        case WM_USER + 300: {
            // 使用配置檔案中的延遲設定
            Sleep(g_state.refocusDelay);
            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            return 0;
        }
        
        case WM_KILLFOCUS: {
            // 失去焦點時立即重新設置前置
            if (g_state.forceStayOnTop) {
                PostMessage(hwnd, WM_USER + 300, 0, 0);
            }
            return 0;
        }	
		
    }
    
    return DefWindowProc(hwnd, msg, wp, lp);
}

// OptimizedUI候選字視窗程序
LRESULT CALLBACK OptimizedCandProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;
            
        case WM_PAINT: { 
            PAINTSTRUCT ps; 
            HDC hdc = BeginPaint(hwnd, &ps); 
            drawCandidate(hwnd, hdc, g_state); 
            EndPaint(hwnd, &ps); 
            return 0; 
        }
            
        case WM_LBUTTONDOWN: { 
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            
            // ★ 優先處理翻頁按鈕點擊
            if (g_state.totalPages > 1) {
                if (isPointInPrevPageButton(x, y, g_state) && g_state.currentPage > 0) {
                    Dictionary::changePage(g_state, -1);
                    return 0;  // 重要：處理完後返回
                }
                
                if (isPointInNextPageButton(x, y, g_state) && g_state.currentPage < g_state.totalPages - 1) {
                    Dictionary::changePage(g_state, 1);
                    return 0;  // 重要：處理完後返回
                }
            }
            
            // 原有的候選字/拖拽邏輯
            int lineHeight = g_state.candidateFontSize + 6;
            int idx = (y - 10) / lineHeight;
            
            // 計算實際顯示的候選字數量
            int startIndex = g_state.currentPage * CANDIDATES_PER_PAGE;
            int endIndex = std::min(startIndex + CANDIDATES_PER_PAGE, (int)g_state.candidates.size());
            int actualCandidateCount = endIndex - startIndex;
            
            // 確保點擊在候選字區域內（不在翻頁控制區域）
            int maxCandidateY = 10 + CANDIDATES_PER_PAGE * lineHeight;
            if (y < maxCandidateY && idx >= 0 && idx < actualCandidateCount) {
                Dictionary::selectCandidate(g_state, idx);
                return 0;
            }
            
            // 如果點擊不在候選字上，則開始拖曳字碼輸入視窗（原有邏輯）
            g_state.dragState.isCandDragging = true;
            SetCapture(hwnd);
            
            POINT pt;
            GetCursorPos(&pt);
            
            // 記錄相對於字碼輸入視窗的偏移量
            if (g_state.hInputWnd) {
                RECT inputRect;
                GetWindowRect(g_state.hInputWnd, &inputRect);
                g_state.dragState.dragOffset.x = pt.x - inputRect.left;
                g_state.dragState.dragOffset.y = pt.y - inputRect.top;
            }
            
            return 0;
        }

        case WM_MOUSEMOVE: {
            int x = LOWORD(lp);
            int y = HIWORD(lp);
            
            // 處理翻頁按鈕懸停效果
            bool needRedraw = false;
            
            if (g_state.totalPages > 1) {
                bool newPrevHover = isPointInPrevPageButton(x, y, g_state) && g_state.currentPage > 0;
                bool newNextHover = isPointInNextPageButton(x, y, g_state) && g_state.currentPage < g_state.totalPages - 1;
                
                if (newPrevHover != g_state.prevPageButtonHover || newNextHover != g_state.nextPageButtonHover) {
                    needRedraw = true;
                }
                
                g_state.prevPageButtonHover = newPrevHover;
                g_state.nextPageButtonHover = newNextHover;
                
                if (needRedraw) {
                    InvalidateRect(hwnd, nullptr, TRUE);
                }
            }
            
            // 原有的拖拽邏輯
            if (g_state.dragState.isCandDragging) {
                POINT pt;
                GetCursorPos(&pt);
                
                // 移動字碼輸入視窗
                if (g_state.hInputWnd) {
                    int newX = pt.x - g_state.dragState.dragOffset.x;
                    int newY = pt.y - g_state.dragState.dragOffset.y;
                    
                    RECT inputRect;
                    GetWindowRect(g_state.hInputWnd, &inputRect);
                    int inputWidth = inputRect.right - inputRect.left;
                    
                    // 移動字碼輸入視窗
                    SetWindowPos(g_state.hInputWnd, HWND_TOPMOST,
                                newX, newY, inputWidth, INPUT_WINDOW_HEIGHT,
                                SWP_NOACTIVATE | SWP_SHOWWINDOW);
                    
                    // 候選字視窗自動跟隨在字碼視窗下方
                    if (g_state.hCandWnd && g_state.showCand) {
                        RECT candRect;
                        GetWindowRect(hwnd, &candRect);
                        int candWidth = candRect.right - candRect.left;
                        int candHeight = candRect.bottom - candRect.top;
                        
                        SetWindowPos(hwnd, HWND_TOPMOST,
                                    newX, newY + INPUT_WINDOW_HEIGHT + WINDOW_SPACING,
                                    candWidth, candHeight,
                                    SWP_NOACTIVATE | SWP_SHOWWINDOW);
                    }
                }
            }
            
            return 0;
        }

        case WM_LBUTTONUP: {
            if (g_state.dragState.isCandDragging) {
                g_state.dragState.isCandDragging = false;
                ReleaseCapture();
                
                // 記錄使用者自定義位置（基於字碼輸入視窗位置）
                if (g_state.hInputWnd) {
                    RECT inputRect;
                    GetWindowRect(g_state.hInputWnd, &inputRect);
                    PositionManager::g_userCandPos.x = inputRect.left;
                    PositionManager::g_userCandPos.y = inputRect.top;
                    PositionManager::g_userCandPos.isValid = true;
                    PositionManager::g_useUserPosition = true;
                    PositionManager::savePositions(g_state);
                    
                    Utils::updateStatus(g_state, L"已切換到使用者位置模式");
                }
                
                return 0;
            }
            break;
        }
    }
    
    return DefWindowProc(hwnd, msg, wp, lp);
}


// 字碼輸入視窗程序實現
LRESULT CALLBACK InputProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            drawInputWindow(hdc, rc, g_state);
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_LBUTTONDOWN: {
            // 開始拖拽字碼輸入視窗
            g_state.dragState.isInputDragging = true;
            SetCapture(hwnd);
            
            POINT pt;
            GetCursorPos(&pt);
            RECT inputRect;
            GetWindowRect(hwnd, &inputRect);
            g_state.dragState.dragOffset.x = pt.x - inputRect.left;
            g_state.dragState.dragOffset.y = pt.y - inputRect.top;
            
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            if (g_state.dragState.isInputDragging) {
                POINT pt;
                GetCursorPos(&pt);
                
                int newX = pt.x - g_state.dragState.dragOffset.x;
                int newY = pt.y - g_state.dragState.dragOffset.y;
                
                RECT inputRect;
                GetWindowRect(hwnd, &inputRect);
                int inputWidth = inputRect.right - inputRect.left;
                
                // 移動字碼輸入視窗
                SetWindowPos(hwnd, HWND_TOPMOST,
                            newX, newY, inputWidth, INPUT_WINDOW_HEIGHT,
                            SWP_NOACTIVATE | SWP_SHOWWINDOW);
                
                // 候選字視窗自動跟隨
                if (g_state.hCandWnd && g_state.showCand) {
                    RECT candRect;
                    GetWindowRect(g_state.hCandWnd, &candRect);
                    int candWidth = candRect.right - candRect.left;
                    int candHeight = candRect.bottom - candRect.top;
                    
                    SetWindowPos(g_state.hCandWnd, HWND_TOPMOST,
                                newX, newY + INPUT_WINDOW_HEIGHT + WINDOW_SPACING,
                                candWidth, candHeight,
                                SWP_NOACTIVATE | SWP_SHOWWINDOW);
                }
                
                return 0;
            }
            break;
        }
        
        case WM_LBUTTONUP: {
            if (g_state.dragState.isInputDragging) {
                g_state.dragState.isInputDragging = false;
                ReleaseCapture();
                
                // 記錄用戶自定義位置
                RECT inputRect;
                GetWindowRect(hwnd, &inputRect);
                PositionManager::g_userCandPos.x = inputRect.left;
                PositionManager::g_userCandPos.y = inputRect.top;
                PositionManager::g_userCandPos.isValid = true;
                PositionManager::g_useUserPosition = true;
                PositionManager::savePositions(g_state);
                
                Utils::updateStatus(g_state, L"已切換到用戶位置模式");
                return 0;
            }
            break;
        }
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// 修復：字碼輸入視窗位置調整
void positionInputWindow(GlobalState& state) {
    if (!state.hInputWnd) {
        return; // 如果視窗不存在就直接返回
    }
    
    // 關鍵修改：只要有輸入就顯示字碼視窗，不管是否有候選字
    if (!state.isInputting || state.input.empty()) {
        ShowWindow(state.hInputWnd, SW_HIDE);
        return;
    }
    
    // 如果有候選字視窗，字碼視窗定位在其上方
    if (state.hCandWnd && state.showCand && IsWindowVisible(state.hCandWnd)) {
        RECT candRect;
        GetWindowRect(state.hCandWnd, &candRect);
        int inputWidth = candRect.right - candRect.left;
        int inputX = candRect.left;
        int inputY = candRect.top - INPUT_WINDOW_HEIGHT - 2;
        
        // 確保字碼視窗在螢幕可見範圍內
        ScreenManager::MonitorInfo monitor = ScreenManager::getMonitorFromPoint({inputX, inputY});
        if (inputY < monitor.workArea.top) {
            inputY = candRect.bottom + 2; // 如果上方放不下，放到候選字視窗下方
        }
        
        SetWindowPos(state.hInputWnd, HWND_TOPMOST, 
                    inputX, inputY, 
                    inputWidth, INPUT_WINDOW_HEIGHT,
                    SWP_NOACTIVATE | SWP_SHOWWINDOW);
    } else {
        // 沒有候選字時，定位到滑鼠附近
        POINT mousePos = PositionManager::getCurrentMousePosition();
        ScreenManager::MonitorInfo monitor = ScreenManager::getMonitorFromPoint(mousePos);
        
        int inputX = mousePos.x;
        int inputY = mousePos.y - 35;
        int inputWidth = state.inputWindowWidth;  // 使用配置的寬度
        
        // 確保視窗在螢幕範圍內
        if (inputX + inputWidth > monitor.workArea.right) {
            inputX = monitor.workArea.right - inputWidth - 10;
        }
        if (inputX < monitor.workArea.left) {
            inputX = monitor.workArea.left + 10;
        }
        if (inputY < monitor.workArea.top) {
            inputY = monitor.workArea.top + 10;
        }
        
            SetWindowPos(state.hInputWnd, HWND_TOPMOST,
                inputX, inputY, 
                state.inputWindowWidth,   // 使用配置的寬度
                state.inputWindowHeight,  // 使用配置的高度
                SWP_NOACTIVATE | SWP_SHOWWINDOW);
    }
    
    InvalidateRect(state.hInputWnd, nullptr, TRUE);
}

} // namespace WindowManager