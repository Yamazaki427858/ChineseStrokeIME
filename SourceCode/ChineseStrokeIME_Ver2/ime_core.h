// ime_core.h - 核心定義與全域狀態 (OptimizedUI支援版)
#ifndef IME_CORE_H
#define IME_CORE_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <ctime>

// 常數定義
const int CANDIDATES_PER_PAGE = 9;
const int FIXED_WIDTH = 380;
const int CHARS_PER_LINE = 10;
const int MIN_HEIGHT = 80;
const int MAX_HEIGHT = 200;
const int LINE_HEIGHT = 20;
const int CONTROL_BAR_HEIGHT = 50;
const int INPUT_WINDOW_HEIGHT = 30;
const int WINDOW_SPACING = 2; 

// OptimizedUI工具列常數
const int TOOLBAR_WIDTH = 290;
const int TOOLBAR_HEIGHT = 35;
const int BUTTON_HEIGHT = 22;
const int SMALL_BUTTON_WIDTH = 35;
const int MODE_BUTTON_WIDTH = 35;

// 位置記憶結構
struct Position {
    int x;
    int y;
    bool isValid;
    Position() : x(0), y(0), isValid(false) {}
};

struct ScreenModePositions {
    Position extendedModePos;
    Position mirroredModePos;
    bool hasExtendedPos;
    bool hasMirroredPos;
    ScreenModePositions() : hasExtendedPos(false), hasMirroredPos(false) {}
};

// 詞頻資訊結構
struct WordInfo {
    int frequency;
    time_t lastUsed;
    int tempCount;
    bool isPermanent;
};

// UI元素位置結構
struct ToolbarElements {
    RECT modeIndicatorRect = {0};
    RECT statusIndicatorRect = {0};
    RECT menuButtonRect = {0};
    RECT bufferButtonRect = {0};    // ⌘暫放按鈕
    RECT restoreButtonRect = {0};
    RECT minimizeButtonRect = {0};
    RECT closeButtonRect = {0};
    
    // 懸停狀態
    bool modeIndicatorHover = false;
    bool menuButtonHover = false;
    bool bufferButtonHover = false;
    bool restoreButtonHover = false;
    bool minimizeButtonHover = false;
    bool closeButtonHover = false;
};

// 拖拽狀態結構
struct DragState {
    bool isToolbarDragging = false;
    bool isInputDragging = false;
    bool isCandDragging = false;
    POINT dragOffset = {0};
};

// UI顏色配置
struct UIColors {
    COLORREF toolbarBgColor = RGB(240,240,240);
    COLORREF toolbarBorderColor = RGB(160,160,160);
    COLORREF modeActiveColor = RGB(0,120,215);
    COLORREF modeInactiveColor = RGB(160,160,160);
    COLORREF statusReadyColor = RGB(0,150,0);
    COLORREF statusErrorColor = RGB(220,50,50);
    COLORREF statusInputColor = RGB(255,165,0);
    COLORREF statusBufferColor = RGB(255,140,0);
    COLORREF buttonHoverColor = RGB(200,200,200);
    COLORREF closeButtonColor = RGB(180,180,180);
    COLORREF bufferButtonActiveColor = RGB(255,165,0);
    COLORREF bufferButtonInactiveColor = RGB(180,180,180);
};

// 全域狀態結構
struct GlobalState {
    // 視窗控制代碼
    HWND hWnd = NULL;
    HWND hCandWnd = NULL;
    HWND hBufferWnd = NULL;
	HWND hInputWnd = NULL;
	
	// 候選字視窗翻頁按鈕
    RECT prevPageButtonRect = {0};
    RECT nextPageButtonRect = {0};
    RECT pageInfoRect = {0};
    
    bool prevPageButtonHover = false;
    bool nextPageButtonHover = false;
    
    // 輸入狀態
    std::wstring input = L"";
    std::vector<std::wstring> candidates;
    std::vector<std::wstring> candidateCodes;
    int selected = 0;
    int currentPage = 0;
    int totalPages = 0;
    bool showCand = false;
    bool chineseMode = true;
    bool isInputting = false;
    bool inputError = false;
    bool showPunctMenu = false;
	
	// 文字選取狀態
    bool isSelecting = false;           // 是否正在選取
    int selectionStart = -1;           // 選取起始位置
    int selectionEnd = -1;             // 選取結束位置
    bool hasSelection = false;         // 是否有選取的文字
    POINT selectionStartPoint = {0};   // 選取起始座標
    POINT selectionEndPoint = {0};     // 選取結束座標
    
    // 右鍵選單相關
    bool showContextMenu = false;      // 是否顯示右鍵選單
    RECT contextMenuRect = {0};        // 右鍵選單位置

    
    // 字典資料
    std::map<std::wstring, std::vector<std::wstring>> dict;
    std::map<std::wstring, std::vector<std::wstring>> punct;
    std::map<std::wstring, WordInfo> wordFreq;
    std::map<std::wstring, std::vector<std::wstring>> contextLearning;
    std::wstring lastSelected = L"";
    std::vector<std::wstring> punctCandidates;
    int dictSize = 0;
    
    // 暫放視窗模式
    bool bufferMode = false;
    std::wstring bufferText = L"";
    int bufferCursorPos = 0;
    bool bufferShowCursor = true;
    DWORD bufferCursorBlinkTime = 0;
    bool bufferHasFocus = false;
	
    // 視窗行為設定
    int topmostCheckInterval = 5000;  // 前置檢查間隔
    bool forceStayOnTop = true;       // 是否強制前置
    int refocusDelay = 50;            // 重新聚焦延遲	
    
    // Shift鍵狀態
    bool shiftPressed = false;
    bool shiftUsedForCombo = false;
    DWORD shiftPressTime = 0;
    
    // 介面狀態
    bool isDragging = false;  // 保留向後兼容
    POINT dragStartPoint = {0};
    std::wstring statusInfo = L"就緒";
    
    // OptimizedUI狀態
    ToolbarElements toolbarElements;
    DragState dragState;
    UIColors uiColors;
    bool useOptimizedUI = true;  // 控制是否使用OptimizedUI風格
    
    // 按鈕狀態 (保留原有，向後兼容)
    RECT closeButtonRect = {0};
    RECT modeButtonRect = {0};
    RECT creditsButtonRect = {0};
    RECT refreshButtonRect = {0};
    RECT bufferButtonRect = {0};
    RECT sendButtonRect = {0};
    RECT clearButtonRect = {0};
    RECT saveButtonRect = {0};
    
    bool closeButtonHover = false;
    bool modeButtonHover = false;
    bool creditsButtonHover = false;
    bool refreshButtonHover = false;
    bool bufferButtonHover = false;
    bool sendButtonHover = false;
    bool clearButtonHover = false;
    bool saveButtonHover = false;
    
    // 顏色設定 (保留原有，向後兼容)
    COLORREF bgColor = RGB(240,240,240);
    COLORREF textColor = RGB(0,0,0);
    COLORREF selColor = RGB(0,120,215);
    COLORREF selBgColor = RGB(230,240,250);
    COLORREF errorColor = RGB(220,50,50);
    COLORREF closeButtonColor = RGB(220,50,50);
    COLORREF closeButtonHoverColor = RGB(255,70,70);
    COLORREF modeButtonColor = RGB(100,50,200);
    COLORREF modeButtonHoverColor = RGB(120,70,220);
    COLORREF creditsButtonColor = RGB(200,150,50);
    COLORREF creditsButtonHoverColor = RGB(220,170,70);
    COLORREF refreshButtonColor = RGB(50,150,50);
    COLORREF refreshButtonHoverColor = RGB(70,170,70);
    COLORREF candidateBackgroundColor = RGB(255,255,255);
    COLORREF candidateTextColor = RGB(0,0,0);
    COLORREF selectedCandidateBackgroundColor = RGB(230,240,250);
    COLORREF selectedCandidateTextColor = RGB(0,120,215);
    COLORREF bufferBackgroundColor = RGB(255,255,255);
    COLORREF bufferTextColor = RGB(0,0,0);
    COLORREF bufferCursorColor = RGB(0,0,0);
    
    // 字碼輸入視窗顏色設定 (MOVED FROM GLOBAL TO STRUCT)
    COLORREF inputBackgroundColor = RGB(255,255,255);
    COLORREF inputTextColor = RGB(0,0,0);
    COLORREF inputErrorTextColor = RGB(220,50,50);
    COLORREF inputHintTextColor = RGB(128,128,128);
    COLORREF inputBorderColor = RGB(128,128,128);
    
    // 字型設定
    int fontSize = 16;
    std::wstring fontName = L"Microsoft JhengHei";
    int candidateFontSize = 18;
    std::wstring candidateFontName = L"Microsoft JhengHei";
    int bufferFontSize = 14;
    std::wstring bufferFontName = L"Microsoft JhengHei";
    
    // 字碼輸入視窗字型設定 (MOVED FROM GLOBAL TO STRUCT)
    int inputFontSize = 16;
    std::wstring inputFontName = L"Microsoft JhengHei";
    
    // 視窗尺寸
    int windowWidth = 580;  // 非OptimizedUI模式使用
    int windowHeight = 70;
    int candidateWidth = 500;
    int candidateHeight = 320;
    
    // 字碼輸入視窗尺寸設定 (MOVED FROM GLOBAL TO STRUCT)
    int inputWindowWidth = 400;
    int inputWindowHeight = 30;
};

// 工具函數命名空間
namespace Utils {
    std::wstring utf8ToWstr(const std::string& str);
    std::string wstrToUtf8(const std::wstring& ws);
    void updateStatus(GlobalState& state, const std::wstring& msg);
    bool isPunctuation(const std::wstring& word);
    COLORREF parseColorFromString(const std::string& colorStr);
    
    // OptimizedUI工具函數
    bool isPointInRect(int x, int y, const RECT& rect);
}

// 全域狀態變數宣告
extern GlobalState g_state;
extern HHOOK g_hKeyboardHook;

#endif // IME_CORE_H