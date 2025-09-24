// OptimizedChineseStrokeIME.cpp - 優化版中文筆劃輸入法
#include <windows.h>
#include <shellapi.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>
#include <cmath>

using namespace std;

// ========== 全域變數 ==========
HWND g_hWnd = NULL;
HWND g_hInputWnd = NULL;
HWND g_hCandWnd = NULL;
HHOOK g_hKeyboardHook = NULL;
NOTIFYICONDATA g_nid = {0};
bool g_isMinimized = false;



// ========== 多螢幕支援 ==========
struct MonitorInfo {
    HMONITOR hMonitor;
    RECT rect;
    RECT workArea;
    bool isPrimary;
};

vector<MonitorInfo> g_monitors;

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    MONITORINFOEX mi;
    mi.cbSize = sizeof(MONITORINFOEX);
    
    if (GetMonitorInfo(hMonitor, &mi)) {
        MonitorInfo info;
        info.hMonitor = hMonitor;
        info.rect = mi.rcMonitor;
        info.workArea = mi.rcWork;
        info.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
        
        g_monitors.push_back(info);
    }
    return TRUE;
}

void update_monitor_info() {
    g_monitors.clear();
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
}

MonitorInfo get_monitor_from_point(POINT pt) {
    HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    
    for (const auto& monitor : g_monitors) {
        if (monitor.hMonitor == hMon) {
            return monitor;
        }
    }
    
    // 回退到主螢幕
    for (const auto& monitor : g_monitors) {
        if (monitor.isPrimary) {
            return monitor;
        }
    }
    
    return g_monitors.empty() ? MonitorInfo{} : g_monitors[0];
}

bool is_point_in_any_monitor(POINT pt) {
    for (const auto& monitor : g_monitors) {
        if (pt.x >= monitor.rect.left && pt.x <= monitor.rect.right &&
            pt.y >= monitor.rect.top && pt.y <= monitor.rect.bottom) {
            return true;
        }
    }
    return false;
}


bool is_extended_mode() {
    update_monitor_info();
    return g_monitors.size() > 1;
}

// 安全的螢幕資訊獲取函數
RECT get_safe_primary_screen() {
    RECT safeRect = {0, 0, 1920, 1080}; // 預設安全值
    
    update_monitor_info();
    
    if (g_monitors.size() <= 1) {
        // 鏡像模式：使用系統工作區域
        if (SystemParametersInfo(SPI_GETWORKAREA, 0, &safeRect, 0)) {
            return safeRect;
        }
    } else {
        // 延伸模式：使用多螢幕邏輯
        for (const auto& monitor : g_monitors) {
            if (monitor.isPrimary) {
                return monitor.workArea;
            }
        }
    }
    
    // 回退方案：使用GetSystemMetrics
    safeRect.right = GetSystemMetrics(SM_CXSCREEN);
    safeRect.bottom = GetSystemMetrics(SM_CYSCREEN);
    
    return safeRect;
}

// ========== 增強的螢幕模式檢測 ==========
// 更精確的鏡像模式檢測
bool is_truly_mirrored_mode() {
    update_monitor_info();
    
    if (g_monitors.size() <= 1) return true;
    
    // 檢查是否所有螢幕都有相同的解析度（鏡像模式特徵）
    if (g_monitors.size() >= 2) {
        RECT firstRect = g_monitors[0].rect;
        for (size_t i = 1; i < g_monitors.size(); i++) {
            RECT currentRect = g_monitors[i].rect;
            if ((currentRect.right - currentRect.left) != (firstRect.right - firstRect.left) ||
                (currentRect.bottom - currentRect.top) != (firstRect.bottom - firstRect.top)) {
                return false; // 不同解析度，確實是延伸模式
            }
        }
        return true; // 相同解析度，可能是鏡像模式
    }
    
    return false;
}

bool is_mirrored_mode() {
    return is_truly_mirrored_mode();
}

// 座標有效性檢查
bool is_coordinate_valid_in_current_mode(int x, int y) {
    update_monitor_info();
    
    for (const auto& monitor : g_monitors) {
        if (x >= monitor.workArea.left && x <= monitor.workArea.right &&
            y >= monitor.workArea.top && y <= monitor.workArea.bottom) {
            return true;
        }
    }
    return false;
}


// 輸入系統
wstring g_input = L"";
vector<wstring> g_candidates;
vector<wstring> g_candidateCodes;
map<wstring, vector<wstring>> g_dict;
map<wstring, vector<wstring>> g_punct;

// 增強的頻率系統
struct WordInfo {
    int frequency;
    time_t lastUsed;
    int tempCount;
    bool isPermanent;
};

map<wstring, WordInfo> g_wordFreq;
wstring g_lastSelected = L"";

int g_selected = 0;
int g_currentPage = 0;
int g_totalPages = 0;
const int CANDIDATES_PER_PAGE = 9;
bool g_showCand = false;
bool g_chineseMode = true;
bool g_isInputting = false;
bool g_inputError = false;
bool g_showPunctMenu = false;
bool g_isToolbarDragging = false;

// Shift鍵狀態
bool g_shiftPressed = false;
bool g_shiftUsedForCombo = false;
DWORD g_shiftPressTime = 0;

// UI元素位置（與附件代碼保持一致）
RECT g_modeIndicatorRect = {0};
RECT g_statusIndicatorRect = {0};
RECT g_menuButtonRect = {0};
RECT g_restoreButtonRect = {0};
RECT g_minimizeButtonRect = {0};
RECT g_closeButtonRect = {0};

bool g_modeIndicatorHover = false;
bool g_menuButtonHover = false;
bool g_restoreButtonHover = false;
bool g_minimizeButtonHover = false;
bool g_closeButtonHover = false;

vector<wstring> g_punctCandidates;

// 優化的位置管理
struct Position {
    int x, y;
    bool isValid;
    Position() : x(0), y(0), isValid(false) {}
    Position(int _x, int _y) : x(_x), y(_y), isValid(true) {}
};

struct ScreenModePositions {
    Position extendedModePos;    // 延伸模式位置
    Position mirroredModePos;    // 鏡像模式位置
    bool hasExtendedPos = false;
    bool hasMirroredPos = false;
};

Position g_toolbarPos;
Position g_userInputPos;
Position g_userCandPos;
bool g_useUserPosition = false;

ScreenModePositions g_screenModePositions;


// 配色設定（與附件代碼保持一致）
COLORREF g_toolbarBgColor = RGB(240,240,240);
COLORREF g_toolbarBorderColor = RGB(160,160,160);
COLORREF g_modeActiveColor = RGB(0,120,215);
COLORREF g_modeInactiveColor = RGB(160,160,160);
COLORREF g_statusReadyColor = RGB(0,150,0);
COLORREF g_statusErrorColor = RGB(220,50,50);
COLORREF g_statusInputColor = RGB(255,165,0);
COLORREF g_buttonHoverColor = RGB(200,200,200);
COLORREF g_closeButtonColor = RGB(180,180,180);

COLORREF g_candidateBackgroundColor = RGB(255,255,255);
COLORREF g_candidateTextColor = RGB(0,0,0);
COLORREF g_selectedCandidateBackgroundColor = RGB(51,153,255);
COLORREF g_selectedCandidateTextColor = RGB(255,255,255);
COLORREF g_candidateBorderColor = RGB(128,128,128);

COLORREF g_inputBackgroundColor = RGB(255,255,255);
COLORREF g_inputTextColor = RGB(0,0,0);
COLORREF g_inputBorderColor = RGB(128,128,128);

int g_candidateFontSize = 14;
wstring g_candidateFontName = L"Microsoft JhengHei";
int g_inputFontSize = 14;
wstring g_inputFontName = L"Microsoft JhengHei";

wstring g_statusInfo = L"優化版輸入法已就緒";

// 視窗大小常數（與附件代碼保持一致）
const int TOOLBAR_WIDTH = 250;
const int TOOLBAR_HEIGHT = 35;
const int MIN_INPUT_WIDTH = 200;
const int MAX_INPUT_WIDTH = 600;
const int MIN_CAND_WIDTH = 300;
const int INPUT_WINDOW_HEIGHT = 40;
const int WINDOW_SPACING = 2;
const int PAGE_BUTTON_HEIGHT = 20; // 新增：分頁按鈕高度

// 配置參數
int g_verticalOffset = 25;

// ========== 工具函數 ==========
wstring utf8_to_wstr(const string& str) {
    if (str.empty()) return wstring();
    int sz = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
    if (sz <= 0) return wstring();
    wstring res(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &res[0], sz);
    return res;
}

string wstr_to_utf8(const wstring& ws) {
    if (ws.empty()) return string();
    int sz = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    if (sz <= 0) return string();
    string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), &res[0], sz, nullptr, nullptr);
    return res;
}

void update_status(const wstring& msg) {
    g_statusInfo = msg;
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
}

// ========== 優化的視窗寬度計算 ==========
int calculate_optimal_window_width() {
    int baseWidth = MIN_INPUT_WIDTH;
    
    // 根據輸入長度計算基礎寬度
    if (!g_input.empty()) {
        baseWidth = max(baseWidth, 120 + (int)g_input.length() * 16);
    }
    
    // 根據候選字內容計算所需寬度
    if (!g_candidates.empty()) {
        int maxContentWidth = 0;
        for (size_t i = 0; i < min(g_candidates.size(), (size_t)CANDIDATES_PER_PAGE); ++i) {
            int contentWidth = 60; // 序號和間距
            contentWidth += (int)g_candidates[i].length() * 18; // 候選字
            
            if (i < g_candidateCodes.size()) {
                contentWidth += 30 + (int)g_candidateCodes[i].length() * 10; // 編碼
            }
            
            // 學習狀態標記
            if (g_wordFreq.find(g_candidates[i]) != g_wordFreq.end()) {
                contentWidth += 30;
            }
            
            maxContentWidth = max(maxContentWidth, contentWidth);
        }
        baseWidth = max(baseWidth, maxContentWidth + 20);
    }
    
    // 確保分頁控制有足夠空間
    if (g_totalPages > 1) {
        baseWidth = max(baseWidth, 150);
    }
    
    
    return max(MIN_CAND_WIDTH, min(baseWidth, MAX_INPUT_WIDTH));
}

// ========== 優化的候選字視窗高度計算 ==========
int calculate_candidate_window_height() {
    if (!g_showCand || g_candidates.empty()) return 0;
    
    int lineHeight = g_candidateFontSize + 8;
    int contentLines = min(CANDIDATES_PER_PAGE, (int)g_candidates.size());
    int baseHeight = 16; // 上下邊距
    

    
    // 候選字列表高度
    baseHeight += contentLines * lineHeight;
    
    // 分頁控制高度
    if (g_totalPages > 1) {
        baseHeight += PAGE_BUTTON_HEIGHT + 10;
    }
    
    return baseHeight;
}

// ========== 多螢幕兼容的定位邏輯 ==========
Position get_current_mouse_position() {
    Position pos;
    GetCursorPos((POINT*)&pos);
    
    // 檢查滑鼠是否在任何螢幕範圍內
    if (!is_point_in_any_monitor({pos.x, pos.y})) {
        // 如果不在任何螢幕內，使用主螢幕中央
        for (const auto& monitor : g_monitors) {
            if (monitor.isPrimary) {
                pos.x = monitor.rect.left + (monitor.rect.right - monitor.rect.left) / 2;
                pos.y = monitor.rect.top + (monitor.rect.bottom - monitor.rect.top) / 2;
                break;
            }
        }
    }
    
    pos.y += g_verticalOffset;
    pos.isValid = true;
    
    return pos;
}

// ========== 多螢幕兼容的統一視窗定位 ==========
void position_windows_optimized() {
    if (!g_isInputting) return;
    
    update_monitor_info();
    
    int unifiedWidth = calculate_optimal_window_width();
    int candHeight = calculate_candidate_window_height();
    
    Position basePos;
    
    if (g_useUserPosition && g_userInputPos.isValid) {
        basePos = g_userInputPos;
    } else {
        basePos = get_current_mouse_position();
        
        RECT screenRect;
        
        // 根據螢幕模式使用不同的邊界檢查策略
        if (is_mirrored_mode()) {
            // 鏡像模式：使用安全的螢幕區域
            screenRect = get_safe_primary_screen();
        } else {
            // 延伸模式：使用多螢幕邏輯
            MonitorInfo currentMonitor = get_monitor_from_point({basePos.x, basePos.y});
            screenRect = currentMonitor.workArea;
        }
        
        // 統一的邊界調整邏輯
        int totalHeight = INPUT_WINDOW_HEIGHT + WINDOW_SPACING + candHeight;
        
        if (basePos.x + unifiedWidth > screenRect.right - 10) {
            basePos.x = screenRect.right - unifiedWidth - 10;
        }
        if (basePos.x < screenRect.left + 10) {
            basePos.x = screenRect.left + 10;
        }
        
        if (basePos.y + totalHeight > screenRect.bottom - 30) {
            int newY = basePos.y - totalHeight - g_verticalOffset;
            if (newY >= screenRect.top + 10) {
                basePos.y = newY;
            } else {
                basePos.y = screenRect.top + (screenRect.bottom - screenRect.top - totalHeight) / 2;
            }
        }
        if (basePos.y < screenRect.top + 10) {
            basePos.y = screenRect.top + 10;
        }
    }
    
    // 定位字碼輸入視窗
    if (g_hInputWnd) {
        SetWindowPos(g_hInputWnd, HWND_TOPMOST, 
                     basePos.x, basePos.y,
                     unifiedWidth, INPUT_WINDOW_HEIGHT,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
// 已經在代碼中處理了這個問題
        InvalidateRect(g_hInputWnd, nullptr, TRUE);
    }
    
    // 定位候選字視窗（固定在字碼視窗正下方）
    if (g_hCandWnd && g_showCand && candHeight > 0) {
        int candY = basePos.y + INPUT_WINDOW_HEIGHT + WINDOW_SPACING;
        
        SetWindowPos(g_hCandWnd, HWND_TOPMOST,
                     basePos.x, candY,
                     unifiedWidth, candHeight,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW);
        InvalidateRect(g_hCandWnd, nullptr, TRUE);
    }
}


// ========== 配置管理 ==========
void save_positions() {
    ofstream config("positions.ini");
    if (!config.is_open()) return;
    
    config << "[Toolbar]" << endl;
    config << "x=" << g_toolbarPos.x << endl;
    config << "y=" << g_toolbarPos.y << endl;
    
    // 根據當前螢幕模式保存到對應區段
    if (is_extended_mode()) {
        config << "[ToolbarExtended]" << endl;
        config << "x=" << g_toolbarPos.x << endl;
        config << "y=" << g_toolbarPos.y << endl;
        
        // 更新記憶中的位置
        g_screenModePositions.extendedModePos = g_toolbarPos;
        g_screenModePositions.hasExtendedPos = true;
    } else {
        config << "[ToolbarMirrored]" << endl;
        config << "x=" << g_toolbarPos.x << endl;
        config << "y=" << g_toolbarPos.y << endl;
        
        // 更新記憶中的位置
        g_screenModePositions.mirroredModePos = g_toolbarPos;
        g_screenModePositions.hasMirroredPos = true;
    }
    
    config << "[UserPosition]" << endl;
    config << "enabled=" << (g_useUserPosition ? "1" : "0") << endl;
    if (g_useUserPosition) {
        config << "input_x=" << g_userInputPos.x << endl;
        config << "input_y=" << g_userInputPos.y << endl;
        config << "cand_x=" << g_userCandPos.x << endl;
        config << "cand_y=" << g_userCandPos.y << endl;
    }
    
    config << "[OptimizedPositioning]" << endl;
    config << "vertical_offset=" << g_verticalOffset << endl;
    
    config.close();
}


// 在 load_positions() 函數的最後添加
void load_positions() {
    ifstream config("positions.ini");
    if (!config.is_open()) {
        // 使用安全的螢幕檢測
        update_monitor_info();
        RECT primaryScreen = get_safe_primary_screen();
        
        g_toolbarPos.x = (int)max((LONG)(primaryScreen.left + 50), 50L);
        g_toolbarPos.y = (int)max((LONG)(primaryScreen.bottom - TOOLBAR_HEIGHT - 80), 50L);
        g_toolbarPos.isValid = true;
        
        save_positions();
        return;
    }
    
    string line, section;
    while (getline(config, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }
        
        size_t eq = line.find('=');
        if (eq == string::npos) continue;
        
        string key = line.substr(0, eq);
        string value = line.substr(eq + 1);
        
        try {
            if (section == "Toolbar") {
                if (key == "x") g_toolbarPos.x = stoi(value);
                else if (key == "y") g_toolbarPos.y = stoi(value);
                g_toolbarPos.isValid = true;
            } else if (section == "UserPosition") {
                if (key == "enabled") g_useUserPosition = (value == "1");
                else if (key == "input_x") g_userInputPos.x = stoi(value);
                else if (key == "input_y") g_userInputPos.y = stoi(value);
                else if (key == "cand_x") g_userCandPos.x = stoi(value);
                else if (key == "cand_y") g_userCandPos.y = stoi(value);
                
                if (g_useUserPosition) {
                    g_userInputPos.isValid = true;
                    g_userCandPos.isValid = true;
                }
            } else if (section == "OptimizedPositioning") {
                if (key == "vertical_offset") g_verticalOffset = stoi(value);
            } else if (section == "ToolbarExtended") {
                // 👆 您找到的代碼片段應該在這裡
                if (key == "x") g_screenModePositions.extendedModePos.x = stoi(value);
                else if (key == "y") g_screenModePositions.extendedModePos.y = stoi(value);
                g_screenModePositions.extendedModePos.isValid = true;
                g_screenModePositions.hasExtendedPos = true;
            } else if (section == "ToolbarMirrored") {
                if (key == "x") g_screenModePositions.mirroredModePos.x = stoi(value);
                else if (key == "y") g_screenModePositions.mirroredModePos.y = stoi(value);
                g_screenModePositions.mirroredModePos.isValid = true;
                g_screenModePositions.hasMirroredPos = true;
            }
        } catch (...) {}
    }
    
    config.close();
    
    // 根據當前螢幕模式自動選擇合適的位置
    if (is_extended_mode() && g_screenModePositions.hasExtendedPos) {
        g_toolbarPos = g_screenModePositions.extendedModePos;
    } else if (is_mirrored_mode() && g_screenModePositions.hasMirroredPos) {
        g_toolbarPos = g_screenModePositions.mirroredModePos;
    }
    
    // 驗證載入的位置是否安全
    RECT currentScreen = get_safe_primary_screen();
    if (g_toolbarPos.x < currentScreen.left || 
        g_toolbarPos.x > currentScreen.right - TOOLBAR_WIDTH ||
        g_toolbarPos.y < currentScreen.top ||
        g_toolbarPos.y > currentScreen.bottom - TOOLBAR_HEIGHT) {
        
        g_toolbarPos.x = currentScreen.left + 50;
        g_toolbarPos.y = currentScreen.bottom - TOOLBAR_HEIGHT - 80;
        save_positions();
    }
    
    // 增強版位置驗證
    update_monitor_info();
    bool positionValid = false;

    for (const auto& monitor : g_monitors) {
        if (g_toolbarPos.x >= monitor.workArea.left && 
            g_toolbarPos.x <= monitor.workArea.right - TOOLBAR_WIDTH &&
            g_toolbarPos.y >= monitor.workArea.top &&
            g_toolbarPos.y <= monitor.workArea.bottom - TOOLBAR_HEIGHT) {
            positionValid = true;
            break;
        }
    }

    if (!positionValid) {
        RECT safeScreen = get_safe_primary_screen();
        g_toolbarPos.x = safeScreen.left + 50;
        g_toolbarPos.y = safeScreen.bottom - TOOLBAR_HEIGHT - 80;
        g_toolbarPos.isValid = true;
        save_positions();
        
        MessageBoxW(NULL, 
            L"偵測到工具列位置在當前螢幕模式下無效，\n"
            L"已自動重置到主螢幕安全位置。", 
            L"位置自動修正", MB_OK | MB_ICONINFORMATION);
    }
}


// ========== 介面配色載入 ==========
COLORREF parse_color(const string& hex) {
    if (hex.empty() || hex[0] != '#') return RGB(255,255,255);
    string colorStr = hex.substr(1);
    if (colorStr.length() != 6) return RGB(255,255,255);
    
    try {
        int r = stoi(colorStr.substr(0,2), nullptr, 16);
        int g = stoi(colorStr.substr(2,2), nullptr, 16); 
        int b = stoi(colorStr.substr(4,2), nullptr, 16);
        return RGB(r,g,b);
    } catch (...) {
        return RGB(255,255,255);
    }
}

void load_interface_config() {
    ifstream config("interface_config.ini");
    if (!config.is_open()) return;
    
    string line, section;
    while (getline(config, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }
        
        size_t eq = line.find('=');
        if (eq == string::npos) continue;
        
        string key = line.substr(0, eq);
        string value = line.substr(eq + 1);
        
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (section == "Colors") {
            COLORREF color = parse_color(value);
            if (key == "candidate_background_color") g_candidateBackgroundColor = color;
            else if (key == "candidate_text_color") g_candidateTextColor = color;
            else if (key == "selected_candidate_background_color") g_selectedCandidateBackgroundColor = color;
            else if (key == "selected_candidate_text_color") g_selectedCandidateTextColor = color;
            else if (key == "input_background_color") g_inputBackgroundColor = color;
            else if (key == "input_text_color") g_inputTextColor = color;
            else if (key == "input_border_color") g_inputBorderColor = color;
            else if (key == "candidate_border_color") g_candidateBorderColor = color;
        }
        else if (section == "Font") {
            if (key == "candidate_font_size") {
                try { g_candidateFontSize = stoi(value); } catch (...) { g_candidateFontSize = 14; }
            }
            else if (key == "candidate_font_name") {
                g_candidateFontName = utf8_to_wstr(value);
            }
            else if (key == "input_font_size") {
                try { g_inputFontSize = stoi(value); } catch (...) { g_inputFontSize = 14; }
            }
            else if (key == "input_font_name") {
                g_inputFontName = utf8_to_wstr(value);
            }
        }
    }
    config.close();
}


// ========== 字典載入 ==========
void load_dict() {
    g_dict.clear();
    ifstream fin("Zi-Ma-Biao.txt");
    if (!fin.is_open()) {
        // 內建測試字典
        g_dict[L"u"] = {L"一"};
        g_dict[L"i"] = {L"丨"};
        g_dict[L"o"] = {L"丿"};
        g_dict[L"j"] = {L"丶"};
        g_dict[L"k"] = {L"乙"};
        g_dict[L"ui"] = {L"工", L"七", L"上"};
        g_dict[L"uj"] = {L"下", L"不", L"丁"};
        g_dict[L"uio"] = {L"中", L"小", L"大"};
        g_dict[L"uioj"] = {L"木", L"水", L"火"};
        g_dict[L"uioji"] = {L"林", L"森", L"樹"};
        g_dict[L"ujk"] = {L"人", L"入", L"八"};
        g_dict[L"uik"] = {L"土", L"士", L"十"};
        g_dict[L"uiojik"] = {L"森林茂密", L"綠樹成蔭"};
        g_dict[L"uiojikuo"] = {L"測試長字碼功能"};
        update_status(L"使用內建測試字典");
        return;
    }
    
    string line;
    int count = 0;
    while (getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t tab = line.find('\t');
        if (tab == string::npos) continue;
        
        string character = line.substr(0, tab);
        string code = line.substr(tab + 1);
        
        wstring wchar = utf8_to_wstr(character);
        wstring wcode = utf8_to_wstr(code);
        
        if (!wchar.empty() && !wcode.empty()) {
            g_dict[wcode].push_back(wchar);
            count++;
        }
    }
    fin.close();
    update_status(L"載入字典：" + to_wstring(count) + L" 個字");
}

void load_punctuator() {
    g_punct.clear();
    g_punct[L","] = {L"，", L","};
    g_punct[L"."] = {L"。", L"."};
    g_punct[L"?"] = {L"？", L"?"};
    g_punct[L"!"] = {L"！", L"!"};
    g_punct[L":"] = {L"：", L":"};
    g_punct[L";"] = {L"；", L";"};
    g_punct[L"("] = {L"（", L"("};
    g_punct[L")"] = {L"）", L")"};
    g_punct[L"["] = {L"「", L"[", L"［", L"["};
    g_punct[L"]"] = {L"」", L"]", L"］", L"]"};
    g_punct[L"{"] = {L"『", L"{"};
    g_punct[L"}"] = {L"』", L"}"};
    g_punct[L" "] = {L" "};
    g_punct[L"<"] = {L"《", L"<"};
    g_punct[L">"] = {L"》", L">"};
    g_punct[L"/"] = {L"／", L"/"};
    g_punct[L"'"] = {L"、", L"'"};
    g_punct[L"-"] = {L"－", L"-"};
    g_punct[L"_"] = {L"＿", L"_"};
    g_punct[L"="] = {L"＝", L"="};
    g_punct[L"\\"] = {L"＼", L"\\"};
    g_punct[L"|"] = {L"｜", L"|"}; 
    g_punct[L"~"] = {L"～", L"~"}; 
    g_punct[L"`"] = {L"`", L"`"};
    g_punct[L"^"] = {L"⌃", L"^"};
    g_punct[L"&"] = {L"＆", L"&"}; 
    g_punct[L"*"] = {L"＊", L"*"}; 
    g_punct[L"+"] = {L"＋", L"+"};
    g_punct[L"#"] = {L"＃", L"#"};
    g_punct[L"@"] = {L"＠", L"@"};   
    g_punct[L"$"] = {L"＄", L"$"}; 
    g_punct[L"%"] = {L"％", L"%"};
    g_punct[L"\""] = {L"＂", L"\""};
}

void load_punct_menu() {
    ifstream fin("punct_menu.txt");
    if (!fin.is_open()) {
        // 使用內建預設標點符號
        g_punctCandidates = { 
            L"※", L"✓", L"，", L"。", L"？", L"！", L"：", L"；", 
            L"（", L"）", L"「", L"」", L"【", L"】", L"『", L"』", 
            L"《", L"》", L"〈", L"〉", L"、", L"·", L"－", L"—", 
            L"……", L""", L""", L"'", L"'", L"｜", L"＼", L"／", 
            L"～", L"＿", L"￥", L"％", L"＃", L"＠", L"｛", L"｝" 
        };
        return;
    }
    
    g_punctCandidates.clear();
    string line;
    while (getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        wstring punct = utf8_to_wstr(line);
        if (!punct.empty()) {
            g_punctCandidates.push_back(punct);
        }
    }
    fin.close();
}


// ========== 用戶字典學習系統 ==========
bool is_punctuation(const wstring& word) {
    if (word.empty()) return false;
    wstring punctuations = L"，。？！：；（）「」【】『』《》〈〉、·－—……""''｜＼／～＿￥％＃＠｛｝"
                          L",.?!:;()[]{}\"'<>/\\-_@#$%^&*+=|`~ 　";
    for (wchar_t ch : word) {
        if (punctuations.find(ch) == wstring::npos && ch != L' ' && ch != L'\t') {
            return false;
        }
    }
    return true;
}

void load_userdict() {
    g_wordFreq.clear();
    ifstream fin("user_dict.txt");
    if (!fin.is_open()) {
        update_status(L"初始化用戶字典");
        return;
    }
    
    string line;
    int count = 0;
    time_t now = time(nullptr);
    
    try {
        while (getline(fin, line)) {
            if (line.empty() || line[0] == '#') continue;
            vector<string> parts;
            stringstream ss(line);
            string part;
            while (getline(ss, part, '\t')) {
                parts.push_back(part);
            }
            if (parts.size() >= 2) {
                wstring character = utf8_to_wstr(parts[0]);
                int freq = (parts.size() >= 3) ? stoi(parts[2]) : 1;
                bool isPerm = (parts.size() >= 4) ? (parts[3] == "permanent") : false;
                if (!character.empty()) {
                    g_wordFreq[character] = {freq, now, max(3, freq), isPerm};
                    count++;
                }
            }
        }
    } catch (...) {}
    fin.close();
    update_status(L"載入用戶字典：" + to_wstring(count) + L" 個記錄");
}

void save_userdict() {
    try {
        ofstream fout("user_dict.txt");
        if (!fout.is_open()) return;
        fout << "# 用戶字典 - 自動生成" << endl;
        fout << "# 格式：詞語<TAB><TAB>使用頻率<TAB>狀態" << endl;
        for (const auto& pair : g_wordFreq) {
            string status = pair.second.isPermanent ? "permanent" : "temp";
            fout << wstr_to_utf8(pair.first) << "\t\t" << pair.second.frequency << "\t" << status << endl;
        }
        fout.close();
    } catch (...) {}
}

void learn_word(const wstring& word) {
    if (is_punctuation(word) || word.empty()) return;
    
    time_t now = time(nullptr);
    if (g_wordFreq.find(word) == g_wordFreq.end()) {
        g_wordFreq[word] = {1, now, 1, false};
    } else {
        WordInfo& info = g_wordFreq[word];
        info.frequency++;
        info.lastUsed = now;
        if (!info.isPermanent) {
            info.tempCount++;
            if (info.tempCount >= 3) {
                info.isPermanent = true;
                update_status(L"詞語已加入永久詞庫：" + word);
            }
        }
    }
    g_lastSelected = word;
}

double get_word_score(const wstring& word, const wstring& code) {
    double score = (10.0 - code.length()) * 2.0;
    if (g_wordFreq.find(word) != g_wordFreq.end()) {
        const WordInfo& info = g_wordFreq[word];
        score += info.frequency * 1.0;
        if (info.isPermanent) score += 5.0;
    }
    return score;
}

void sort_candidates_by_score() {
    vector<pair<wstring, wstring>> candidatePairs;
    for (size_t i = 0; i < g_candidates.size(); i++) {
        candidatePairs.push_back(make_pair(g_candidates[i], g_candidateCodes[i]));
    }
    sort(candidatePairs.begin(), candidatePairs.end(), 
         [](const pair<wstring, wstring>& a, const pair<wstring, wstring>& b) {
        return get_word_score(a.first, a.second) > get_word_score(b.first, b.second);
    });
    g_candidates.clear();
    g_candidateCodes.clear();
    for (const auto& pair : candidatePairs) {
        g_candidates.push_back(pair.first);
        g_candidateCodes.push_back(pair.second);
    }
}

// ========== 通配符搜尋 ==========
bool wildcard_match(const wstring& pattern, const wstring& text) {
    int pLen = pattern.length();
    int tLen = text.length();
    
    vector<vector<bool>> dp(tLen + 1, vector<bool>(pLen + 1, false));
    dp[0][0] = true;
    
    for (int j = 1; j <= pLen; j++) {
        if (pattern[j-1] == L'*') {
            dp[0][j] = dp[0][j-1];
        }
    }
    
    for (int i = 1; i <= tLen; i++) {
        for (int j = 1; j <= pLen; j++) {
            if (pattern[j-1] == L'*') {
                dp[i][j] = dp[i-1][j] || dp[i][j-1];
            } else if (pattern[j-1] == text[i-1]) {
                dp[i][j] = dp[i-1][j-1];
            }
        }
    }
    
    return dp[tLen][pLen];
}

// ========== 容錯輸入處理 ==========
bool enhanced_validate_input(const wstring& input) {
    if (input.empty()) return true;
    if (input.length() > 30) return false;
    
    int validCharCount = 0;
    for (wchar_t ch : input) {
        if (ch == L'u' || ch == L'i' || ch == L'o' || ch == L'j' || ch == L'k' || ch == L'*') {
            validCharCount++;
        }
    }
    
    return validCharCount > 0;
}

wstring filter_valid_chars(const wstring& input) {
    wstring filtered;
    for (wchar_t ch : input) {
        if (ch == L'u' || ch == L'i' || ch == L'o' || ch == L'j' || ch == L'k' || ch == L'*') {
            filtered += ch;
        }
    }
    return filtered;
}

void update_candidates_enhanced() {
    g_candidates.clear();
    g_candidateCodes.clear();
    g_selected = 0;
    g_currentPage = 0;
    g_inputError = false;

    if (g_input.empty()) { 
        g_showCand = false;
        g_isInputting = false;
        if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
        if (g_hInputWnd) ShowWindow(g_hInputWnd, SW_HIDE);
        update_status(g_chineseMode ? L"中文模式（全形標點）" : L"英文模式（半形標點）");
        return; 
    }

    if (!enhanced_validate_input(g_input)) {
        g_inputError = true;
        g_showCand = false;
        g_isInputting = true;
        if (g_hInputWnd) ShowWindow(g_hInputWnd, SW_SHOW);
        if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
        update_status(L"字碼過長：建議使用(3+3)搜尋或清除重新輸入");
        return;
    }

    wstring filteredInput = filter_valid_chars(g_input);
    
    if (filteredInput.empty()) {
        g_inputError = true;
        g_showCand = false;
        g_isInputting = true;
        if (g_hInputWnd) ShowWindow(g_hInputWnd, SW_SHOW);
        if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
        update_status(L"請輸入有效字碼：uiojk或*");
        return;
    }

    bool hasWildcard = filteredInput.find(L'*') != wstring::npos;
    if (hasWildcard) {
        for (const auto& pair : g_dict) {
            if (wildcard_match(filteredInput, pair.first)) {
                for (const auto& character : pair.second) {
                    g_candidates.push_back(character);
                    g_candidateCodes.push_back(pair.first);
                }
            }
        }
    } else {
        if (g_dict.count(filteredInput)) {
            for (const auto& character : g_dict[filteredInput]) {
                g_candidates.push_back(character);
                g_candidateCodes.push_back(filteredInput);
            }
        }
        
        int prefixMatchCount = 0;
        const int MAX_PREFIX_MATCHES = 50;
        for (const auto& pair : g_dict) {
            if (prefixMatchCount >= MAX_PREFIX_MATCHES) break;
            if (pair.first.length() > filteredInput.length() && 
                pair.first.substr(0, filteredInput.length()) == filteredInput) {
                for (const auto& character : pair.second) {
                    if (find(g_candidates.begin(), g_candidates.end(), character) == g_candidates.end()) {
                        g_candidates.push_back(character);
                        g_candidateCodes.push_back(pair.first);
                        prefixMatchCount++;
                        if (prefixMatchCount >= MAX_PREFIX_MATCHES) break;
                    }
                }
            }
        }
        
        if (filteredInput.length() > 8 && g_candidates.empty()) {
            wstring first3 = filteredInput.substr(0, min(3, (int)filteredInput.length()));
            wstring last3;
            if (filteredInput.length() >= 6) {
                last3 = filteredInput.substr(filteredInput.length() - 3);
            } else if (filteredInput.length() > 3) {
                last3 = filteredInput.substr(3);
            }
            wstring searchPattern = first3 + L"*" + last3;
            for (const auto& pair : g_dict) {
                if (wildcard_match(searchPattern, pair.first)) {
                    for (const auto& character : pair.second) {
                        g_candidates.push_back(character);
                        g_candidateCodes.push_back(pair.first);
                    }
                }
            }
        }
    }

    sort_candidates_by_score();
    g_totalPages = (g_candidates.size() + CANDIDATES_PER_PAGE - 1) / CANDIDATES_PER_PAGE;
    g_showCand = !g_candidates.empty();
    g_isInputting = true;

    position_windows_optimized();

    wstring statusMsg;
    if (filteredInput != g_input) {
        statusMsg = L"容錯搜尋(" + filteredInput + L")：" + to_wstring(g_candidates.size()) + L"個候選字";
    } else {
        statusMsg = (hasWildcard ? L"(3+3)搜尋" : (filteredInput.length() > 8 ? L"長字碼搜尋" : L"智慧搜尋"));
        statusMsg += L"：" + to_wstring(g_candidates.size()) + L"個候選字";
    }
    
    if (filteredInput.length() > 6 && !hasWildcard && g_candidates.empty()) {
        wstring first3 = filteredInput.substr(0, min(3, (int)filteredInput.length()));
        wstring last3;
        if (filteredInput.length() >= 6) {
            last3 = filteredInput.substr(filteredInput.length() - 3);
        } else if (filteredInput.length() > 3) {
            last3 = filteredInput.substr(3);
        }
        if (!last3.empty()) {
            statusMsg += L" | 建議(3+3)：" + first3 + L"*" + last3;
        }
    }
    update_status(statusMsg);
}

wstring get_input_display() {
    wstring display = g_input;
    if (g_showPunctMenu) {
        display = L"標點符號選單";
    } else if (!g_input.empty()) {
        wstring filtered = filter_valid_chars(g_input);
        if (filtered != g_input) {
            display += L" [已過濾: " + filtered + L"]";
        }
        
        if (filtered.length() > 6) {
            wstring first3 = filtered.substr(0, 3);
            wstring last3 = filtered.substr(filtered.length() - 3);
            display += L" (建議: " + first3 + L"*" + last3 + L")";
        } else if (filtered.length() > 3) {
            display += L" (可用*號搜尋)";
        }
    }
    return display;
}

void send_text(const wstring& text) {
    if (text.empty()) return;
    
    for (wchar_t ch : text) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0;
        input.ki.wScan = ch;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        SendInput(1, &input, sizeof(INPUT));
        
        input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
        Sleep(5);
    }
}

void toggle_input_mode() {
    g_chineseMode = !g_chineseMode;
    g_input.clear();
    g_candidates.clear();
    g_candidateCodes.clear();
    g_showCand = false;
    g_isInputting = false;
    g_inputError = false;
    g_showPunctMenu = false;
    if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
    if (g_hInputWnd) ShowWindow(g_hInputWnd, SW_HIDE);
    
    wstring modeMsg = g_chineseMode ? L"中文模式（全形標點）" : L"英文模式（半形標點）";
    update_status(L"切換到" + modeMsg);
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
}

// ========== 輸入處理函數 ==========
void show_punct_menu() {
    g_showPunctMenu = true;
    g_candidates = g_punctCandidates;
    g_candidateCodes.clear();
    for (size_t i = 0; i < g_candidates.size(); i++) {
        g_candidateCodes.push_back(L"P");
    }
    g_selected = 0;
    g_currentPage = 0;
    g_totalPages = (g_candidates.size() + CANDIDATES_PER_PAGE - 1) / CANDIDATES_PER_PAGE;
    g_showCand = true;
    g_isInputting = true;
    position_windows_optimized();
    update_status(L"標點符號選單");
}

void process_stroke(DWORD key) {
    if (!g_chineseMode) return;
    if (key == 'P') {
        show_punct_menu();
        return;
    }
    wchar_t inputChar = 0;
    switch (key) {
        case 'U': inputChar = L'u'; break;
        case 'I': inputChar = L'i'; break;
        case 'O': inputChar = L'o'; break;
        case 'J': inputChar = L'j'; break;
        case 'K': inputChar = L'k'; break;
        case 'L': inputChar = L'*'; break;
        case VK_NUMPAD7: inputChar = L'u'; break;
        case VK_NUMPAD8: inputChar = L'i'; break;
        case VK_NUMPAD9: inputChar = L'o'; break;
        case VK_NUMPAD4: inputChar = L'j'; break;
        case VK_NUMPAD5: inputChar = L'k'; break;
        case VK_NUMPAD0: inputChar = L'*'; break;
    }
    if (inputChar) {
        g_input += inputChar;
        update_candidates_enhanced();
        if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
    }
}

void process_punctuator(DWORD key) {
    wstring punctChar = L"";
    bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    
    switch (key) {
        case VK_OEM_COMMA: punctChar = isShiftPressed ? L"<" : L","; break;
        case VK_OEM_PERIOD: punctChar = isShiftPressed ? L">" : L"."; break;
        case VK_OEM_2: punctChar = isShiftPressed ? L"?" : L"/"; break;
        case VK_OEM_1: punctChar = isShiftPressed ? L":" : L";"; break;
        case VK_OEM_4: punctChar = isShiftPressed ? L"{" : L"["; break;
        case VK_OEM_6: punctChar = isShiftPressed ? L"}" : L"]"; break;
        case VK_OEM_7: punctChar = isShiftPressed ? L"\"" : L"'"; break;
        case VK_OEM_MINUS: punctChar = isShiftPressed ? L"_" : L"-"; break;
        case VK_OEM_PLUS: punctChar = isShiftPressed ? L"+" : L"="; break;
        case VK_OEM_5: punctChar = isShiftPressed ? L"|" : L"\\"; break;
        case VK_OEM_3: punctChar = isShiftPressed ? L"~" : L"`"; break;
        case VK_SPACE: punctChar = L" "; break;
        case '1': if (isShiftPressed) punctChar = L"!"; break;
        case '2': if (isShiftPressed) punctChar = L"@"; break;
        case '3': if (isShiftPressed) punctChar = L"#"; break;
        case '4': if (isShiftPressed) punctChar = L"$"; break;
        case '5': if (isShiftPressed) punctChar = L"%"; break;
        case '6': if (isShiftPressed) punctChar = L"^"; break;
        case '7': if (isShiftPressed) punctChar = L"&"; break;
        case '8': if (isShiftPressed) punctChar = L"*"; break;
        case '9': if (isShiftPressed) punctChar = L"("; break;
        case '0': if (isShiftPressed) punctChar = L")"; break;
    }
    
    if (!punctChar.empty() && g_punct.count(punctChar)) {
        vector<wstring> options = g_punct[punctChar];
        if (!options.empty()) {
            wstring selectedPunct;
            
            if (punctChar == L" ") {
                selectedPunct = L" ";
            } else if (punctChar == L"'") {
                selectedPunct = g_chineseMode ? L"、" : L"'";
            } else {
                selectedPunct = g_chineseMode ? options[0] : 
                    (options.size() > 1 ? options[1] : options[0]);
            }
            
            send_text(selectedPunct);
            
            wstring modeDesc = g_chineseMode ? L"全形" : L"半形";
            update_status(L"輸入" + modeDesc + L"標點：" + selectedPunct);
        }
    }
}

void change_page(int direction) {
    if (!g_showCand || g_totalPages <= 1) return;
    if (direction > 0 && g_currentPage < g_totalPages - 1) {
        g_currentPage++;
        g_selected = 0;
    } else if (direction < 0 && g_currentPage > 0) {
        g_currentPage--;
        g_selected = 0;
    }
    if (g_hCandWnd) InvalidateRect(g_hCandWnd, nullptr, TRUE);
}

void select_candidate(int idx) {
    int actualIndex = g_currentPage * CANDIDATES_PER_PAGE + idx;
    if (actualIndex < 0 || actualIndex >= (int)g_candidates.size()) return;
    wstring selected = g_candidates[actualIndex];
    send_text(selected);
    
    if (!g_showPunctMenu) {
        learn_word(selected);
        save_userdict();
    }
    
    g_input.clear();
    g_candidates.clear();
    g_candidateCodes.clear();
    g_showCand = false;
    g_isInputting = false;
    g_inputError = false;
    g_showPunctMenu = false;
    if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
    if (g_hInputWnd) ShowWindow(g_hInputWnd, SW_HIDE);
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
}

// ========== 優化的繪製函數 ==========
void draw_toolbar(HDC hdc, RECT rc) {
    // 背景（與附件保持一致）
    HBRUSH hBg = CreateSolidBrush(g_toolbarBgColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    
    // 邊框
    HPEN hPen = CreatePen(PS_SOLID, 1, g_toolbarBorderColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, rc.right, rc.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    SetBkMode(hdc, TRANSPARENT);
    
    int x = 5;
    int y = (rc.bottom - 22) / 2;
    
    HFONT hFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft JhengHei");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    SetTextColor(hdc, RGB(60, 60, 60));
    TextOutW(hdc, x, y + 3, L"筆劃", 2);
    x += 50;
    
    // 模式指示器（與附件保持一致的大小）
    g_modeIndicatorRect = {x, y, x + 35, y + 22};
    COLORREF modeColor = g_chineseMode ? g_modeActiveColor : g_modeInactiveColor;
    if (g_modeIndicatorHover) modeColor = g_buttonHoverColor;
    
    HBRUSH hModeBrush = CreateSolidBrush(modeColor);
    FillRect(hdc, &g_modeIndicatorRect, hModeBrush);
    DeleteObject(hModeBrush);
    
    SetTextColor(hdc, RGB(255, 255, 255));
    wstring modeText = g_chineseMode ? L"中" : L"EN";
    RECT modeTextRect = g_modeIndicatorRect;
    DrawTextW(hdc, modeText.c_str(), -1, &modeTextRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    x += 40;
    
    // 狀態指示器
    g_statusIndicatorRect = {x, y + 8, x + 6, y + 14};
    COLORREF statusColor = g_statusReadyColor;
    if (g_inputError) statusColor = g_statusErrorColor;
    else if (g_isInputting) statusColor = g_statusInputColor;
    else if (g_useUserPosition) statusColor = RGB(255, 140, 0);
    
    HBRUSH hStatusBrush = CreateSolidBrush(statusColor);
    HPEN hStatusPen = CreatePen(PS_SOLID, 1, RGB(100, 100, 100));
    SelectObject(hdc, hStatusPen);
    SelectObject(hdc, hStatusBrush);
    Ellipse(hdc, g_statusIndicatorRect.left, g_statusIndicatorRect.top,
            g_statusIndicatorRect.right, g_statusIndicatorRect.bottom);
    DeleteObject(hStatusBrush);
    DeleteObject(hStatusPen);
    x += 12;
    
    // 選單按鈕（與附件保持一致的大小）
    g_menuButtonRect = {x, y, x + 40, y + 22};
    if (g_menuButtonHover) {
        HBRUSH hMenuBrush = CreateSolidBrush(g_buttonHoverColor);
        FillRect(hdc, &g_menuButtonRect, hMenuBrush);
        DeleteObject(hMenuBrush);
    }
    SetTextColor(hdc, RGB(60, 60, 60));
    DrawTextW(hdc, L"☰", -1, &g_menuButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    x += 45;
    
    // 重置按鈕（與附件保持一致的大小）
    g_restoreButtonRect = {x, y, x + 35, y + 22};
	if (g_restoreButtonHover) {
    HBRUSH hResetBrush = CreateSolidBrush(g_buttonHoverColor);
    FillRect(hdc, &g_restoreButtonRect, hResetBrush);
    DeleteObject(hResetBrush);
	}

	HFONT hResetFont = CreateFontW(25, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Microsoft JhengHei");

	HFONT hOldResetFont = (HFONT)SelectObject(hdc, hResetFont);
	SetTextColor(hdc, RGB(60, 60, 60));
	DrawTextW(hdc, L"⿻", -1, &g_restoreButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hdc, hOldResetFont);
	DeleteObject(hResetFont);
	x += 40;
    
    // 最小化按鈕（與附件保持一致的大小）
    g_minimizeButtonRect = {x, y, x + 20, y + 22};
    if (g_minimizeButtonHover) {
        HBRUSH hMinBrush = CreateSolidBrush(g_buttonHoverColor);
        FillRect(hdc, &g_minimizeButtonRect, hMinBrush);
        DeleteObject(hMinBrush);
    }
    SetTextColor(hdc, RGB(60, 60, 60));
    DrawTextW(hdc, L"─", -1, &g_minimizeButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    x += 25;
    
    // 關閉按鈕（與附件保持一致的大小）
    g_closeButtonRect = {x, y, x + 20, y + 22};
    COLORREF closeColor = g_closeButtonHover ? RGB(255, 70, 70) : g_closeButtonColor;
    HBRUSH hCloseBrush = CreateSolidBrush(closeColor);
    FillRect(hdc, &g_closeButtonRect, hCloseBrush);
    DeleteObject(hCloseBrush);
    
    SetTextColor(hdc, RGB(255, 255, 255));
    DrawTextW(hdc, L"×", -1, &g_closeButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);
}


void draw_input(HDC hdc, RECT rc) {
    // 背景
    HBRUSH hBg = CreateSolidBrush(g_inputBackgroundColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    
    // 邊框
    HPEN hPen = CreatePen(PS_SOLID, 1, g_inputBorderColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, rc.right, rc.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, g_inputTextColor);
    
    HFONT hFont = CreateFontW(g_inputFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, g_inputFontName.c_str());
    HFONT hOld = (HFONT)SelectObject(hdc, hFont);
    
    wstring display = get_input_display();
    if (!display.empty()) {
        // 確保文字不會超出視窗邊界
        RECT textRect = {8, 8, rc.right - 8, rc.bottom - 8};
        DrawTextW(hdc, display.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    

    
    SelectObject(hdc, hOld);
    DeleteObject(hFont);
}

void draw_candidates(HDC hdc, RECT rc) {
    if (g_candidates.empty()) return;
    
    // 背景
    HBRUSH hBg = CreateSolidBrush(g_candidateBackgroundColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    
    // 邊框
    HPEN hPen = CreatePen(PS_SOLID, 1, g_candidateBorderColor);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, rc.right, rc.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    SetBkMode(hdc, TRANSPARENT);
    
    HFONT hFont = CreateFontW(g_candidateFontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, g_candidateFontName.c_str());
    HFONT hOld = (HFONT)SelectObject(hdc, hFont);
    
    int lineHeight = g_candidateFontSize + 8;
    int startY = 8;
    

    
    // 候選字列表
    int startIndex = g_currentPage * CANDIDATES_PER_PAGE;
    int endIndex = min(startIndex + CANDIDATES_PER_PAGE, (int)g_candidates.size());
    
    for (int i = 0; i < endIndex - startIndex; ++i) {
        int actualIndex = startIndex + i;
        int yPos = startY + i * lineHeight;
        
        // 選中項背景
        if (i == g_selected) {
            RECT bgRect = {2, yPos - 2, rc.right - 2, yPos + lineHeight - 2};
            HBRUSH hBrush = CreateSolidBrush(g_selectedCandidateBackgroundColor);
            FillRect(hdc, &bgRect, hBrush);
            DeleteObject(hBrush);
            SetTextColor(hdc, g_selectedCandidateTextColor);
        } else {
            SetTextColor(hdc, g_candidateTextColor);
        }
        
        // 構建顯示文字
        wstring txt = to_wstring(i+1) + L". " + g_candidates[actualIndex];
        
        // 學習狀態標記
        if (!g_showPunctMenu && g_wordFreq.find(g_candidates[actualIndex]) != g_wordFreq.end()) {
            const WordInfo& info = g_wordFreq[g_candidates[actualIndex]];
            if (info.isPermanent) {
                txt += L" ★";
            } else {
                txt += L" (" + to_wstring(info.frequency) + L")";
            }
        }
        
        // 編碼顯示
        if (!g_showPunctMenu && actualIndex < g_candidateCodes.size()) {
            txt += L" [" + g_candidateCodes[actualIndex] + L"]";
        }
        
        // 確保文字不會超出邊界
        RECT textRect = {8, yPos, rc.right - 8, yPos + lineHeight};
        DrawTextW(hdc, txt.c_str(), -1, &textRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }
    
    // 優化的分頁控制
    if (g_totalPages > 1) {
        int buttonY = startY + CANDIDATES_PER_PAGE * lineHeight + 5;
        
        SetTextColor(hdc, RGB(80, 80, 80));
        
        // 上一頁按鈕
        RECT upRect = {8, buttonY, 35, buttonY + PAGE_BUTTON_HEIGHT};
        if (g_currentPage > 0) {
            HBRUSH hUpBrush = CreateSolidBrush(RGB(240, 240, 240));
            FillRect(hdc, &upRect, hUpBrush);
            DeleteObject(hUpBrush);
            
            HPEN hUpPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
            SelectObject(hdc, hUpPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, upRect.left, upRect.top, upRect.right, upRect.bottom);
            DeleteObject(hUpPen);
            
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawTextW(hdc, L"▲", -1, &upRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else {
            SetTextColor(hdc, RGB(180, 180, 180));
            DrawTextW(hdc, L"▲", -1, &upRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        
        // 下一頁按鈕
        RECT downRect = {40, buttonY, 67, buttonY + PAGE_BUTTON_HEIGHT};
        if (g_currentPage < g_totalPages - 1) {
            HBRUSH hDownBrush = CreateSolidBrush(RGB(240, 240, 240));
            FillRect(hdc, &downRect, hDownBrush);
            DeleteObject(hDownBrush);
            
            HPEN hDownPen = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
            SelectObject(hdc, hDownPen);
            SelectObject(hdc, GetStockObject(NULL_BRUSH));
            Rectangle(hdc, downRect.left, downRect.top, downRect.right, downRect.bottom);
            DeleteObject(hDownPen);
            
            SetTextColor(hdc, RGB(0, 0, 0));
            DrawTextW(hdc, L"▼", -1, &downRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        } else {
            SetTextColor(hdc, RGB(180, 180, 180));
            DrawTextW(hdc, L"▼", -1, &downRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }
        
        // 分頁信息
        SetTextColor(hdc, RGB(100, 100, 100));
        wstring pageInfo = to_wstring(g_currentPage + 1) + L"/" + to_wstring(g_totalPages);
        pageInfo += L" (共" + to_wstring(g_candidates.size()) + L"個)";
        TextOutW(hdc, 75, buttonY + 4, pageInfo.c_str(), (int)pageInfo.size());
        
        // 操作提示
        SetTextColor(hdc, RGB(120, 120, 120));
        wstring hintText = L"↑↓鍵翻頁 | 1-9選字 | 空格選首字";
        int hintX = rc.right - 200;
        if (hintX < 75 + (int)pageInfo.size() * 8 + 10) {
            hintX = 75 + (int)pageInfo.size() * 8 + 10;
        }
        if (hintX + 180 < rc.right - 5) {
            TextOutW(hdc, hintX, buttonY + 4, hintText.c_str(), (int)hintText.size());
        }
    }
    
    SelectObject(hdc, hOld);
    DeleteObject(hFont);
}

bool is_point_in_rect(int x, int y, const RECT& rect) {
    return (x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom);
}

void show_context_menu(HWND hwnd) {
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
}

void show_about_dialog(HWND hwnd) {
    MessageBoxW(hwnd,
        L"Toolbar版中文筆劃輸入法 Ver1\n\n"
        L"🎯 製作人員\n"
        L"編寫員：Perplexity.ai\n"
        L"測試員：山崎大叔（人類）\n\n"
        L"✨ 主要特色\n"
        L"✓ 免安裝、免Admin權限\n"
        L"✓ 隨身攜帶、即開即用\n"
        L"✓ 多螢幕完全支援\n"
        L"✓ 托盤右鍵快捷功能\n\n"
        L"⌨️ 基本操作\n"
        L"• U I O J K：基本筆劃輸入\n"
        L"• 1-9數字鍵：選擇候選字\n"
        L"• Shift鍵：切換中英文模式\n"
        L"• 右鍵托盤圖示：快捷選單\n\n"
        L"感謝您的使用！", 
        L"關於 - Toolbar版筆劃輸入法",
        MB_OK | MB_ICONINFORMATION);
}

// ========== 配置重新載入功能 ==========
void reload_all_configurations() {
    load_interface_config();
    load_punctuator();
    load_userdict();
    load_positions();
    load_punct_menu();
	
    
    if (g_isInputting) {
        update_candidates_enhanced();
    }
    
    // 重新繪製所有視窗以應用新配置
    if (g_hWnd) {
        InvalidateRect(g_hWnd, NULL, TRUE);
        UpdateWindow(g_hWnd);
    }
    
    if (g_hInputWnd) {
        InvalidateRect(g_hInputWnd, NULL, TRUE);
        UpdateWindow(g_hInputWnd);
    }
    
    if (g_hCandWnd) {
        InvalidateRect(g_hCandWnd, NULL, TRUE);
        UpdateWindow(g_hCandWnd);
    }
    
    update_status(L"介面配置已重新載入");
}




// ========== 系統托盤功能 ==========
void create_tray_icon() {
    g_nid.cbSize = sizeof(NOTIFYICONDATA);
    g_nid.hWnd = g_hWnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_USER + 200;
    g_nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(g_nid.szTip, L"筆劃輸入法");
    Shell_NotifyIcon(NIM_ADD, &g_nid);
}

void remove_tray_icon() {
    Shell_NotifyIcon(NIM_DELETE, &g_nid);
}



void show_from_tray() {
    g_isMinimized = false;
    ShowWindow(g_hWnd, SW_SHOW);
    SetForegroundWindow(g_hWnd);
}

void hide_to_tray() {
    g_isMinimized = true;
    ShowWindow(g_hWnd, SW_HIDE);
}

// ========== 鍵盤鉤子 ==========
LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        DWORD key = pKeyboard->vkCode;

        if (wParam == WM_KEYDOWN) {
            if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT) {
                if (!g_shiftPressed) {
                    g_shiftPressed = true;
                    g_shiftUsedForCombo = false;
                    g_shiftPressTime = GetTickCount();
                }
            } else if (g_shiftPressed && key != VK_SHIFT) {
                g_shiftUsedForCombo = true;
            }
        } else if (wParam == WM_KEYUP) {
            if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT) {
                if (g_shiftPressed) {
                    g_shiftPressed = false;
                    DWORD pressDuration = GetTickCount() - g_shiftPressTime;
                    if (!g_shiftUsedForCombo && pressDuration < 500 && pressDuration > 30) {
                        toggle_input_mode();
                    }
                    g_shiftUsedForCombo = false;
                }
            }
        }
        
        if (wParam == WM_KEYDOWN) {
            bool isStrokeKey = (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L' || key == 'P' ||
                                key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || 
                                key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0);
            
            bool isPunctKey = (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 ||
                   key == VK_OEM_1 || key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 ||
                   key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||
                   key == VK_SPACE ||
                   (key == '1' && g_shiftPressed) || (key == '2' && g_shiftPressed) || 
                   (key == '3' && g_shiftPressed) || (key == '4' && g_shiftPressed) || 
                   (key == '5' && g_shiftPressed) || (key == '6' && g_shiftPressed) || 
                   (key == '7' && g_shiftPressed) || (key == '8' && g_shiftPressed) || 
                   (key == '9' && g_shiftPressed) || (key == '0' && g_shiftPressed));
            
            bool isFunctionKey = ((g_isInputting || g_showPunctMenu) && 
                      (key == VK_SPACE || key == VK_BACK || key == VK_ESCAPE ||
                       key == VK_UP || key == VK_DOWN || 
                       (key >= '1' && key <= '9') ||
                       (key >= VK_NUMPAD1 && key <= VK_NUMPAD9)));

            if (g_chineseMode) {
                if (isStrokeKey || isPunctKey || isFunctionKey) {
                    PostMessage(g_hWnd, WM_USER+100, key, 0);
                    return 1;
                }
            } else {
                if (isPunctKey || isFunctionKey) {
                     PostMessage(g_hWnd, WM_USER+100, key, 0);
                     return 1;
                }
            }
        }
    }
    
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

// ========== 重啟輸入法功能 ==========
void restart_ime() {
    // 獲取當前執行檔路徑
    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    
    // 儲存當前設定
    save_userdict();
    save_positions();
    
    // 關閉鍵盤鉤子
    if (g_hKeyboardHook) {
        UnhookWindowsHookEx(g_hKeyboardHook);
        g_hKeyboardHook = NULL;
    }
    
    // 移除托盤圖示
    remove_tray_icon();
    
    // 啟動新的程序實例
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(STARTUPINFO);
    
    if (CreateProcess(exePath, NULL, NULL, NULL, FALSE, 
                      0, NULL, NULL, &si, &pi)) {
        // 成功啟動新實例，關閉當前實例
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        
        // 延遲一下確保新程序啟動
        Sleep(500);
        
        // 退出當前程序
        PostQuitMessage(0);
    } else {
        // 啟動失敗，恢復托盤圖示
        create_tray_icon();
        
        // 重新安裝鍵盤鉤子
        HINSTANCE hInstance = GetModuleHandle(NULL);
        g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, 0);
        
        MessageBoxW(NULL, L"重啟輸入法失敗！", L"錯誤", MB_OK | MB_ICONERROR);
    }
}

// ========== 視窗處理程序 ==========
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
		case WM_DISPLAYCHANGE: {
    // 記錄切換前的螢幕模式
    static bool previousExtended = false;
    static bool firstTime = true;
    
    if (firstTime) {
        previousExtended = is_extended_mode();
        firstTime = false;
    }
    
    // 更新螢幕信息
    update_monitor_info();
    
    // 檢測螢幕模式變更
    bool currentExtended = is_extended_mode();
    bool modeChanged = (previousExtended != currentExtended);
    
    if (modeChanged) {
        // 螢幕模式發生變更，需要重新定位工具列
        RECT safeScreen = get_safe_primary_screen();
        
        // 如果從延伸模式切換到鏡像模式
        if (previousExtended && !currentExtended) {
            // 副螢幕座標在鏡像模式下無效，強制移動到主螢幕
            g_toolbarPos.x = safeScreen.left + 50;
            g_toolbarPos.y = safeScreen.bottom - TOOLBAR_HEIGHT - 80;
            
            MessageBoxW(NULL, 
                L"偵測到螢幕模式變更：延伸→同步\n"
                L"工具列已自動移至主螢幕安全位置。", 
                L"螢幕模式變更", MB_OK | MB_ICONINFORMATION);
        }
        // 如果從鏡像模式切換到延伸模式
        else if (!previousExtended && currentExtended) {
            // 檢查是否有延伸模式的記憶位置
            if (g_screenModePositions.hasExtendedPos) {
                g_toolbarPos = g_screenModePositions.extendedModePos;
                
                // 驗證記憶位置是否仍然有效
                bool positionValid = false;
                for (const auto& monitor : g_monitors) {
                    if (g_toolbarPos.x >= monitor.workArea.left && 
                        g_toolbarPos.x <= monitor.workArea.right - TOOLBAR_WIDTH &&
                        g_toolbarPos.y >= monitor.workArea.top &&
                        g_toolbarPos.y <= monitor.workArea.bottom - TOOLBAR_HEIGHT) {
                        positionValid = true;
                        break;
                    }
                }
                
                if (!positionValid) {
                    g_toolbarPos.x = safeScreen.left + 50;
                    g_toolbarPos.y = safeScreen.bottom - TOOLBAR_HEIGHT - 80;
                }
            }
        }
        
        // 立即更新工具列位置
        SetWindowPos(g_hWnd, NULL, g_toolbarPos.x, g_toolbarPos.y, 
                     0, 0, SWP_NOSIZE | SWP_NOZORDER);
        save_positions();
        
        // 更新記錄的螢幕模式
        previousExtended = currentExtended;
    } else {
        // 沒有模式變更，但仍需驗證位置
        bool toolbarVisible = false;
        for (const auto& monitor : g_monitors) {
            if (g_toolbarPos.x >= monitor.workArea.left && 
                g_toolbarPos.x <= monitor.workArea.right - TOOLBAR_WIDTH &&
                g_toolbarPos.y >= monitor.workArea.top &&
                g_toolbarPos.y <= monitor.workArea.bottom - TOOLBAR_HEIGHT) {
                toolbarVisible = true;
                break;
            }
        }
        
        if (!toolbarVisible) {
            RECT safeScreen = get_safe_primary_screen();
            g_toolbarPos.x = safeScreen.left + 50;
            g_toolbarPos.y = safeScreen.bottom - TOOLBAR_HEIGHT - 80;
            
            SetWindowPos(g_hWnd, NULL, g_toolbarPos.x, g_toolbarPos.y, 
                         0, 0, SWP_NOSIZE | SWP_NOZORDER);
            save_positions();
            
            update_status(L"工具列位置已自動修正");
        }
    }
    
    // 重新定位輸入視窗
    if (g_isInputting) {
        position_windows_optimized();
    }
    
    return 0;
}

        case WM_DESTROY:
            if (g_hKeyboardHook) { 
                UnhookWindowsHookEx(g_hKeyboardHook); 
                g_hKeyboardHook = NULL; 
            }
            remove_tray_icon();
            save_userdict();
            save_positions();
            PostQuitMessage(0); 
            return 0;
            
        case WM_USER + 200:
    if (lp == WM_LBUTTONDOWN) {
        if (g_isMinimized) {
            show_from_tray();
        }
    } else if (lp == WM_RBUTTONDOWN) {
        HMENU hMenu = CreatePopupMenu();
        
        // 基本功能
        if (g_isMinimized) {
            AppendMenu(hMenu, MF_STRING, 2001, L"🔍 顯示輸入法");
        } else {
            AppendMenu(hMenu, MF_STRING, 2001, L"📌 顯示/置前");
        }
        
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        
        // 模式切換
        wstring modeText = g_chineseMode ? L"🔄 切換到英文模式" : L"🔄 切換到中文模式";
        AppendMenu(hMenu, MF_STRING, 2004, modeText.c_str());
        
        // 快捷功能
        AppendMenu(hMenu, MF_STRING, 2005, L"📝 標點符號選單");
        AppendMenu(hMenu, MF_STRING, 2006, L"🔄 重新載入配置");
        
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        
        // 位置控制
        AppendMenu(hMenu, MF_STRING, 2002, L"📍 重置為滑鼠跟隨");
        if (g_useUserPosition) {
            AppendMenu(hMenu, MF_STRING, 2007, L"🎯 取消固定位置");
        }
        
        AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
        
        // 系統功能
        AppendMenu(hMenu, MF_STRING, 2008, L"ℹ️ 關於");
		// 重啟功能
        AppendMenu(hMenu, MF_STRING, 2009, L"🔄 重啟輸入法");
        AppendMenu(hMenu, MF_STRING, 2003, L"❌ 關閉輸入法");
        
        POINT pt;
        GetCursorPos(&pt);
        SetForegroundWindow(hwnd);
        TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
        DestroyMenu(hMenu);
    }
    return 0;
            
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case 1001: show_punct_menu(); break;
                case 1002: 
                    load_dict();
                    update_status(L"字典已重新載入");
                    break;
                case 1005:
                    load_dict();
                    load_punctuator();
                    load_userdict();
                    load_punct_menu();
                    update_status(L"所有配置已重新載入");
                    break;
                case 1003:
					show_about_dialog(hwnd);
						break;

                case 2001: show_from_tray(); break;
				case 2002:
					g_useUserPosition = false;
					save_positions();
					update_status(L"已重置為滑鼠跟隨模式");
				break;
				case 2003: PostMessage(hwnd, WM_CLOSE, 0, 0); break;
        
				case 2004:  // 模式切換
					toggle_input_mode();
				break;
				case 2005:  // 標點符號選單
					show_punct_menu();
				break;
				case 2006:  // 重新載入配置
					reload_all_configurations();
					MessageBoxW(hwnd, L"配置已重新載入！", L"提示", MB_OK | MB_ICONINFORMATION);
				break;
				case 2007:  // 取消固定位置
					g_useUserPosition = false;
					save_positions();
					update_status(L"已取消固定位置");
				break;
				case 2009:  // 重啟輸入法
					if (MessageBoxW(hwnd, L"確定要重啟輸入法嗎？\n\n重啟後將保留所有設定和學習記錄。", 
                    L"確認重啟", MB_YESNO | MB_ICONQUESTION) == IDYES) {
					restart_ime();
			}
				break;
				
				case 2008:  // 關於
				show_about_dialog(hwnd);
				break;
            }
            return 0;
            
        case WM_USER+100: {
            DWORD key = (DWORD)wp;
            
            if (g_chineseMode && (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L' || key == 'P' ||
                                  key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || 
                                  key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0)) { 
                process_stroke(key); 
                return 0; 
            }
            
            if (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 || 
                key == VK_OEM_1 || key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 ||
                key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||
                key == VK_SPACE || 
                (key == '1' && (GetKeyState(VK_SHIFT) & 0x8000)) || 
                (key == '2' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                (key == '3' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                (key == '4' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                (key == '5' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                (key == '6' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                (key == '7' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                (key == '8' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
                (key == '9' && (GetKeyState(VK_SHIFT) & 0x8000)) || 
                (key == '0' && (GetKeyState(VK_SHIFT) & 0x8000))) { 
                process_punctuator(key); 
                return 0; 
            }
            
            if (key == VK_DOWN) { change_page(1); return 0; }
            if (key == VK_UP) { change_page(-1); return 0; }
            if (key >= '1' && key <= '9') { select_candidate(key - '1'); return 0; }
            
            if (key >= VK_NUMPAD1 && key <= VK_NUMPAD9) { 
                select_candidate(key - VK_NUMPAD1); 
                return 0; 
            }
            
            if (key == VK_BACK) { 
                if (!g_input.empty()) { 
                    g_input.pop_back(); 
                    update_candidates_enhanced(); 
                    InvalidateRect(hwnd, nullptr, TRUE); 
                } 
                return 0; 
            }
            if (key == VK_SPACE) { select_candidate(0); return 0; }
            if (key == VK_ESCAPE) { 
                g_input.clear(); 
                g_candidates.clear(); 
                g_candidateCodes.clear(); 
                g_showCand = false; 
                g_isInputting = false; 
                g_inputError = false; 
                g_showPunctMenu = false; 
                if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
                if (g_hInputWnd) ShowWindow(g_hInputWnd, SW_HIDE);
                update_status(L"已清除輸入"); 
                InvalidateRect(hwnd, nullptr, TRUE); 
                return 0; 
            }
            
            break; 
        }
        
        case WM_PAINT: { 
            PAINTSTRUCT ps; 
            HDC hdc = BeginPaint(hwnd, &ps); 
            RECT rc;
            GetClientRect(hwnd, &rc);
            draw_toolbar(hdc, rc); 
            EndPaint(hwnd, &ps); 
            return 0; 
        }
        
                case WM_LBUTTONDOWN: {
            int x = LOWORD(lp); 
            int y = HIWORD(lp);
            
            if (is_point_in_rect(x, y, g_closeButtonRect)) { 
                if (MessageBoxW(hwnd, L"確定要關閉筆劃輸入法嗎？", L"確認關閉", MB_YESNO | MB_ICONQUESTION) == IDYES) { 
                    PostMessage(hwnd, WM_CLOSE, 0, 0); 
                } 
                return 0; 
            }
            if (is_point_in_rect(x, y, g_modeIndicatorRect)) { 
                toggle_input_mode(); 
                return 0; 
            }
            if (is_point_in_rect(x, y, g_menuButtonRect)) { 
                show_context_menu(hwnd);
                return 0; 
            }
            if (is_point_in_rect(x, y, g_restoreButtonRect)) {
                g_useUserPosition = false;
                save_positions();
                update_status(L"已恢復滑鼠跟隨模式");
                return 0;
            }
            if (is_point_in_rect(x, y, g_minimizeButtonRect)) { 
                hide_to_tray(); 
                return 0; 
            }
            
            // 工具列拖動
            g_isToolbarDragging = true;
            SetCapture(hwnd);
            return 0;
        }

        
        case WM_MOUSEMOVE: {
            if (g_isToolbarDragging) {
                POINT pt;
                GetCursorPos(&pt);
                SetWindowPos(hwnd, NULL, pt.x - 150, pt.y - 15, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
                return 0;
            }
            else {
                int x = LOWORD(lp); 
                int y = HIWORD(lp);
                bool needRedraw = false;
                
                bool newModeHover = is_point_in_rect(x, y, g_modeIndicatorRect);
                bool newMenuHover = is_point_in_rect(x, y, g_menuButtonRect);
                bool newRestoreHover = is_point_in_rect(x, y, g_restoreButtonRect);
                bool newMinimizeHover = is_point_in_rect(x, y, g_minimizeButtonRect);
                bool newCloseHover = is_point_in_rect(x, y, g_closeButtonRect);
                
                if (newModeHover != g_modeIndicatorHover || newMenuHover != g_menuButtonHover ||
                    newRestoreHover != g_restoreButtonHover || newMinimizeHover != g_minimizeButtonHover || 
                    newCloseHover != g_closeButtonHover) {
                    needRedraw = true;
                }
                
                g_modeIndicatorHover = newModeHover;
                g_menuButtonHover = newMenuHover;
                g_restoreButtonHover = newRestoreHover;
                g_minimizeButtonHover = newMinimizeHover;
                g_closeButtonHover = newCloseHover;
                
                if (needRedraw) {
                    InvalidateRect(hwnd, nullptr, TRUE);
                }
            }
            break;
        }

        case WM_LBUTTONUP: { 
            if (g_isToolbarDragging) { 
                g_isToolbarDragging = false; 
                ReleaseCapture(); 
                
                RECT rect;
                GetWindowRect(hwnd, &rect);
                g_toolbarPos.x = rect.left;
                g_toolbarPos.y = rect.top;
                save_positions();
                return 0; 
            } 
            break; 
        }
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK InputWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: { 
            PAINTSTRUCT ps; 
            HDC hdc = BeginPaint(hwnd, &ps); 
            RECT rc;
            GetClientRect(hwnd, &rc);
            draw_input(hdc, rc); 
            EndPaint(hwnd, &ps); 
            return 0; 
        }
        case WM_LBUTTONDOWN: {
            g_isToolbarDragging = true;
            SetCapture(hwnd);
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (g_isToolbarDragging) {
                POINT pt;
                GetCursorPos(&pt);
                
                // 計算統一寬度和高度
                int unifiedWidth = calculate_optimal_window_width();
                int candHeight = calculate_candidate_window_height();
                
                // 同步移動兩個視窗，嚴格保持相對位置
                SetWindowPos(hwnd, NULL, pt.x - 50, pt.y - 10, 
                             unifiedWidth, INPUT_WINDOW_HEIGHT, SWP_NOZORDER);
                
                if (g_hCandWnd && g_showCand) {
                    SetWindowPos(g_hCandWnd, NULL, pt.x - 50, pt.y - 10 + INPUT_WINDOW_HEIGHT + WINDOW_SPACING,
                                 unifiedWidth, candHeight, SWP_NOZORDER);
                }
                return 0;
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (g_isToolbarDragging) {
                g_isToolbarDragging = false;
                ReleaseCapture();
                
                // 記錄用戶自定義位置
                RECT inputRect, candRect;
                GetWindowRect(hwnd, &inputRect);
                g_userInputPos.x = inputRect.left;
                g_userInputPos.y = inputRect.top;
                g_userInputPos.isValid = true;
                
                if (g_hCandWnd && GetWindowRect(g_hCandWnd, &candRect)) {
                    g_userCandPos.x = candRect.left;
                    g_userCandPos.y = candRect.top;
                    g_userCandPos.isValid = true;
                }
                
                g_useUserPosition = true;
                save_positions();
                update_status(L"已切換到用戶位置模式");
                return 0;
            }
            break;
        }
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

LRESULT CALLBACK CandProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_ERASEBKGND: return 1;
        case WM_PAINT: { 
            PAINTSTRUCT ps; 
            HDC hdc = BeginPaint(hwnd, &ps); 
            RECT rc;
            GetClientRect(hwnd, &rc);
            draw_candidates(hdc, rc); 
            EndPaint(hwnd, &ps); 
            return 0; 
        }
        case WM_LBUTTONDOWN: { 
			int x = LOWORD(lp);
			int y = HIWORD(lp); 
			int lineHeight = g_candidateFontSize + 8;
			int startY = 8;  // 移除調試模式判斷，直接使用8
    
			// 檢查是否點擊候選字
			int idx = (y - startY) / lineHeight; 
			if (idx >= 0 && idx < CANDIDATES_PER_PAGE && idx < (int)g_candidates.size()) { 
				select_candidate(idx); 
				return 0;
    }
            
            // 檢查分頁按鈕點擊
            if (g_totalPages > 1) {
                int buttonY = startY + CANDIDATES_PER_PAGE * lineHeight + 5;
                if (y >= buttonY && y <= buttonY + PAGE_BUTTON_HEIGHT) {
                    if (x >= 8 && x <= 35 && g_currentPage > 0) {
                        change_page(-1);
                        return 0;
                    } else if (x >= 40 && x <= 67 && g_currentPage < g_totalPages - 1) {
                        change_page(1);
                        return 0;
                    }
                }
            }
            
            // 開始拖拽
            g_isToolbarDragging = true;
            SetCapture(hwnd);
            return 0;
        }
        case WM_MOUSEMOVE: {
            if (g_isToolbarDragging) {
                POINT pt;
                GetCursorPos(&pt);
                
                // 計算統一寬度和高度
                int unifiedWidth = calculate_optimal_window_width();
                int candHeight = calculate_candidate_window_height();
                
                // 同步移動兩個視窗，嚴格保持相對位置
                SetWindowPos(g_hInputWnd, NULL, pt.x - 50, pt.y - 30 - INPUT_WINDOW_HEIGHT,
                             unifiedWidth, INPUT_WINDOW_HEIGHT, SWP_NOZORDER);
                SetWindowPos(hwnd, NULL, pt.x - 50, pt.y - 30, 
                             unifiedWidth, candHeight, SWP_NOZORDER);
                return 0;
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (g_isToolbarDragging) {
                g_isToolbarDragging = false;
                ReleaseCapture();
                
                // 記錄用戶自定義位置
                RECT inputRect, candRect;
                GetWindowRect(g_hInputWnd, &inputRect);
                GetWindowRect(hwnd, &candRect);
                
                g_userInputPos.x = inputRect.left;
                g_userInputPos.y = inputRect.top;
                g_userInputPos.isValid = true;
                
                g_userCandPos.x = candRect.left;
                g_userCandPos.y = candRect.top;
                g_userCandPos.isValid = true;
                
                g_useUserPosition = true;
                save_positions();
                update_status(L"已切換到用戶位置模式");
                return 0;
            }
            break;
        }
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

// ========== 主函數 ==========
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    try {
        // 載入所有配置和字典
        load_positions();
        load_dict();
        load_punctuator();
        load_punct_menu();
        load_userdict();
		load_interface_config(); 
        update_monitor_info();
		
               // 註冊視窗類別
        WNDCLASSW wc = {0};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = L"OptimizedStrokeIME";
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassW(&wc)) { 
            MessageBoxW(NULL, L"無法註冊主視窗類別", L"錯誤", MB_OK | MB_ICONERROR); 
            return 1; 
        }
        
        // 註冊輸入視窗類別
        wc.lpfnWndProc = InputWndProc;
        wc.lpszClassName = L"OptimizedInput";
        if (!RegisterClassW(&wc)) {
            MessageBoxW(NULL, L"無法註冊輸入視窗類別", L"錯誤", MB_OK | MB_ICONERROR);
            return 1;
        }
        
        // 註冊候選字視窗類別
        wc.lpfnWndProc = CandProc;
        wc.lpszClassName = L"OptimizedCand";
        if (!RegisterClassW(&wc)) {
            MessageBoxW(NULL, L"無法註冊候選字視窗類別", L"錯誤", MB_OK | MB_ICONERROR);
            return 1;
        }
		
        if (!g_toolbarPos.isValid) {
            RECT safeScreen = get_safe_primary_screen();
            g_toolbarPos.x = safeScreen.left + 50;
            g_toolbarPos.y = safeScreen.bottom - TOOLBAR_HEIGHT - 80;
            g_toolbarPos.isValid = true;
            save_positions();
        }
		

// 確保工具列位置在可見範圍內
RECT currentScreen = get_safe_primary_screen();
g_toolbarPos.x = (int)max((LONG)currentScreen.left, min((LONG)g_toolbarPos.x, currentScreen.right - TOOLBAR_WIDTH));
g_toolbarPos.y = (int)max((LONG)currentScreen.top, min((LONG)g_toolbarPos.y, currentScreen.bottom - TOOLBAR_HEIGHT));
       
		
        // 建立主工具列視窗
        g_hWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"OptimizedStrokeIME", 
            L"筆劃輸入法", WS_POPUP | WS_BORDER, 
            g_toolbarPos.x, g_toolbarPos.y, TOOLBAR_WIDTH, TOOLBAR_HEIGHT, 
            NULL, NULL, hInstance, NULL);
        if (!g_hWnd) { 
            MessageBoxW(NULL, L"無法建立主工具列視窗", L"錯誤", MB_OK | MB_ICONERROR); 
            return 1; 
        }
        
        // 建立字碼輸入視窗
        g_hInputWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"OptimizedInput", L"", 
            WS_POPUP | WS_BORDER, 100, 100, MIN_INPUT_WIDTH, INPUT_WINDOW_HEIGHT, 
            NULL, NULL, hInstance, NULL);
        if (!g_hInputWnd) { 
            MessageBoxW(NULL, L"無法建立字碼輸入視窗", L"錯誤", MB_OK | MB_ICONERROR); 
            return 1; 
        }
        
        // 建立候選字視窗
        g_hCandWnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, L"OptimizedCand", L"", 
            WS_POPUP | WS_BORDER, 100, 130, MIN_CAND_WIDTH, 200, 
            NULL, NULL, hInstance, NULL);
        if (!g_hCandWnd) { 
            MessageBoxW(NULL, L"無法建立候選字視窗", L"錯誤", MB_OK | MB_ICONERROR); 
            return 1; 
        }
        
        // 安裝鍵盤鉤子
        g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hInstance, 0);
        if (!g_hKeyboardHook) { 
            MessageBoxW(NULL, L"無法安裝鍵盤鉤子\n\n可能原因：\n• 缺少管理員權限\n• 防毒軟體阻擋\n• 系統安全設定限制\n\n輸入法仍可使用，但可能影響全域按鍵擷取功能。", 
                       L"警告", MB_OK | MB_ICONWARNING); 
        }
        
        // 建立系統托盤圖示
        create_tray_icon();
        
        // 顯示主工具列視窗
        ShowWindow(g_hWnd, SW_SHOW);
        UpdateWindow(g_hWnd);
        
        // 初始隱藏輸入相關視窗
        ShowWindow(g_hInputWnd, SW_HIDE);
        ShowWindow(g_hCandWnd, SW_HIDE);
        
        // 設置初始狀態
        update_status(g_chineseMode ? L"中文模式（全形標點）" : L"英文模式（半形標點）");
        
        
        // 主訊息循環
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // 程式結束前的資源清理
        if (g_hKeyboardHook) {
            UnhookWindowsHookEx(g_hKeyboardHook);
            g_hKeyboardHook = NULL;
        }
        
        // 移除系統托盤圖示
        remove_tray_icon();
        
        // 儲存用戶設定
        save_userdict();
        save_positions();
        
       
        
        return (int)msg.wParam;
        
    } catch (const exception& e) {
        // C++ 標準例外處理
        string errorMsg = "程式發生異常：" + string(e.what());
        errorMsg += "\n\n請檢查：";
        errorMsg += "\n• 字典檔案是否存在";
        errorMsg += "\n• 設定檔案是否可寫入";
        errorMsg += "\n• 系統權限是否足夠";
        
        MessageBoxA(NULL, errorMsg.c_str(), "程式異常", MB_OK | MB_ICONERROR);
        
        // 嘗試清理資源
        if (g_hKeyboardHook) {
            UnhookWindowsHookEx(g_hKeyboardHook);
        }
        remove_tray_icon();
        
        return 1;
    } catch (...) {
        // 捕獲所有其他異常
        MessageBoxW(NULL, 
            L"程式發生未知異常！\n\n"
            L"可能原因：\n"
            L"• 系統記憶體不足\n"
            L"• 檔案存取權限問題\n"
            L"• 相容性問題\n"
            L"• 系統API呼叫失敗\n\n"
            L"建議解決方案：\n"
            L"• 以管理員身分執行\n"
            L"• 檢查防毒軟體設定\n"
            L"• 確認Windows版本相容性\n"
            L"• 重新啟動系統後再試", 
            L"嚴重錯誤", MB_OK | MB_ICONERROR);
        
        // 嘗試清理資源
        if (g_hKeyboardHook) {
            UnhookWindowsHookEx(g_hKeyboardHook);
        }
        remove_tray_icon();
        
        return 1;
    }
}

