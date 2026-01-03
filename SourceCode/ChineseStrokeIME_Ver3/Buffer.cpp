#include "Buffer.h"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
using namespace std;

// ========== 外部函數聲明（這些函數在主程式中定義）==========
extern wstring utf8_to_wstr(const string& str);
extern string wstr_to_utf8(const wstring& wstr);
extern void update_status(const wstring& msg);
extern void send_text_direct_unicode(const wstring& text);
extern HWND g_hWnd;
extern bool g_isInputting;
extern bool g_bufferButtonHover;

// ========== 全域變數 ==========
bool g_bufferMode = false;
HWND g_hBufferWnd = NULL;
wstring g_bufferText = L"";
int g_bufferCursorPos = 0;
bool g_bufferShowCursor = true;
DWORD g_bufferCursorBlinkTime = 0;
bool g_bufferHasFocus = false;

const int BUFFER_FIXED_WIDTH = 320;
const int BUFFER_CHARS_PER_LINE = 10;
const int BUFFER_MIN_HEIGHT = 80;
const int BUFFER_MAX_HEIGHT = 200;
const int BUFFER_LINE_HEIGHT = 20;
const int BUFFER_CONTROL_BAR_HEIGHT = 30;

// ========== Position ==========
struct Position g_bufferFixedOffset = {0, 5, false};

// ========== 修復：添加缺少的拖拽變數 ==========
bool g_isBufferDragging = false;
POINT g_bufferDragStart = {0, 0};
POINT g_bufferDragOffset = {0, 0};

// ========== UI ==========
RECT g_sendButtonRect = {0};
RECT g_clearButtonRect = {0};
RECT g_saveButtonRect = {0};
bool g_sendButtonHover = false;
bool g_clearButtonHover = false;
bool g_saveButtonHover = false;
RECT g_bufferButtonRect = {0};

COLORREF g_bufferBackgroundColor = RGB(255, 255, 255);
COLORREF g_bufferTextColor = RGB(0, 0, 0);
COLORREF g_bufferCursorColor = RGB(0, 0, 0);
COLORREF g_bufferButtonColor = RGB(240, 240, 240);
COLORREF g_bufferButtonHoverColor = RGB(220, 220, 220);

// ========== 前向聲明 ==========
void draw_buffer_window(HDC hdc, RECT& clientRect);
void draw_buffer_button(HDC hdc);
LRESULT CALLBACK BufferWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ========== 基礎函數 ==========
void init_buffer_window() {
    g_bufferFixedOffset.x = 0;
    g_bufferFixedOffset.y = 5;
    g_bufferFixedOffset.isValid = false;

    WNDCLASS wc = {0};
    wc.lpfnWndProc = BufferWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"BufferWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_IBEAM);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&wc);

    g_hBufferWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        L"BufferWindow", L"",
        WS_POPUP | WS_BORDER | WS_SYSMENU,
        100, 100, BUFFER_FIXED_WIDTH, BUFFER_MIN_HEIGHT,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );
}

void cleanup_buffer_window() {
    if (g_hBufferWnd) {
        save_buffer_to_file();
        DestroyWindow(g_hBufferWnd);
        g_hBufferWnd = NULL;
    }
    g_bufferMode = false;
    g_bufferHasFocus = false;
}

void toggle_buffer_mode() {
    g_bufferMode = !g_bufferMode;
    if (g_bufferMode) {
        if (!g_hBufferWnd) {
            init_buffer_window();
        }
        if (g_hBufferWnd) {
            load_buffer_from_file();
            update_buffer_position();
            ShowWindow(g_hBufferWnd, SW_SHOW);
            SetFocus(g_hBufferWnd);
            g_bufferHasFocus = true;
            update_status(L"暫放模式 - 開啟");
        }
    } else {
        if (g_hBufferWnd && IsWindowVisible(g_hBufferWnd)) {
            save_buffer_to_file();
            ShowWindow(g_hBufferWnd, SW_HIDE);
            g_bufferHasFocus = false;
            SetFocus(g_hWnd);
            update_status(L"暫放模式 - 關閉");
        }
    }
}

void update_buffer_position() {
    if (g_hBufferWnd && g_hWnd && IsWindowVisible(g_hBufferWnd)) {
        RECT toolbarRect;
        GetWindowRect(g_hWnd, &toolbarRect);
        SetWindowPos(g_hBufferWnd, HWND_TOPMOST, 
                     toolbarRect.left + g_bufferFixedOffset.x,
                     toolbarRect.bottom + g_bufferFixedOffset.y,
                     0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

int calculate_buffer_window_height() {
    if (g_bufferText.empty()) return BUFFER_MIN_HEIGHT;
    
    int lineCount = (g_bufferText.length() / BUFFER_CHARS_PER_LINE) + 1;
    int contentHeight = lineCount * BUFFER_LINE_HEIGHT;
    int totalHeight = contentHeight + BUFFER_CONTROL_BAR_HEIGHT + 20;
    
    return min(max(totalHeight, BUFFER_MIN_HEIGHT), BUFFER_MAX_HEIGHT);
}

bool is_buffer_window_active() {
    if (!g_bufferMode || !g_hBufferWnd) return false;
    
    HWND foregroundWnd = GetForegroundWindow();
    return IsWindowVisible(g_hBufferWnd) && g_bufferHasFocus && 
           (foregroundWnd == g_hWnd || foregroundWnd == g_hBufferWnd);
}

// ========== 全域變數：記住上次活動的視窗 ==========
static HWND g_lastActiveWindow = NULL;

// ========== 修復的send_buffer_content函數 ==========
void send_buffer_content() {
    if (g_bufferText.empty()) {
        update_status(L"暫放區為空");
        return;
    }
    
    // 步驟1：記住原始狀態
    bool originalBufferMode = g_bufferMode;
    bool originalHasFocus = g_bufferHasFocus;
    
    // 步驟2：尋找目標視窗
    HWND targetWindow = NULL;
    
    // 首先嘗試使用記住的視窗
    if (g_lastActiveWindow && IsWindow(g_lastActiveWindow) && IsWindowVisible(g_lastActiveWindow)) {
        targetWindow = g_lastActiveWindow;
    } else {
        // 枚舉尋找合適的目標視窗
        EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
            if (IsWindowVisible(hwnd) && !IsIconic(hwnd)) {
                wchar_t className[256];
                GetClassName(hwnd, className, 256);
                
                // 排除系統和輸入法視窗
                if (wcscmp(className, L"Shell_TrayWnd") != 0 &&
                    wcscmp(className, L"TaskListThumbnailWnd") != 0 &&
                    wcscmp(className, L"BufferWindow") != 0 &&
                    wcscmp(className, L"OptimizedStrokeIME") != 0 &&
                    wcscmp(className, L"OptimizedInput") != 0 &&
                    wcscmp(className, L"OptimizedCand") != 0) {
                    
                    // 優先選擇文字編輯程式
                    if (wcscmp(className, L"Notepad") == 0 ||
                        wcsstr(className, L"Edit") != NULL ||
                        wcsstr(className, L"Word") != NULL) {
                        *(HWND*)lParam = hwnd;
                        return FALSE; // 找到優先目標，停止枚舉
                    }
                    
                    // 記住第一個找到的合適視窗
                    if (*(HWND*)lParam == NULL) {
                        *(HWND*)lParam = hwnd;
                    }
                }
            }
            return TRUE;
        }, (LPARAM)&targetWindow);
    }
    
    if (!targetWindow) {
        update_status(L"未找到目標程式");
        return;
    }
    
    // 記住這個視窗供下次使用
    g_lastActiveWindow = targetWindow;
    
    // 步驟3：暫時隱藏暫放視窗但不改變模式
    g_bufferHasFocus = false;
    ShowWindow(g_hBufferWnd, SW_HIDE);
    Sleep(200);
    
    // 步驟4：激活目標程式視窗
    // 如果視窗最小化，先還原
    if (IsIconic(targetWindow)) {
        ShowWindow(targetWindow, SW_RESTORE);
        Sleep(200);
    }
    
    // 強制激活目標視窗
    DWORD currentThread = GetCurrentThreadId();
    DWORD targetThread = GetWindowThreadProcessId(targetWindow, NULL);
    
    if (currentThread != targetThread) {
        AttachThreadInput(currentThread, targetThread, TRUE);
    }
    
    SetForegroundWindow(targetWindow);
    SetActiveWindow(targetWindow);
    SetFocus(targetWindow);
    
    if (currentThread != targetThread) {
        AttachThreadInput(currentThread, targetThread, FALSE);
    }
    
    // 等待視窗切換完成
    Sleep(300);
    
    // 確認切換成功
    if (GetForegroundWindow() != targetWindow) {
        // 再試一次
        SetWindowPos(targetWindow, HWND_TOP, 0, 0, 0, 0, 
                     SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
        SetForegroundWindow(targetWindow);
        Sleep(200);
    }
    
    // 步驟5：發送文字到目標程式
    send_text_direct_unicode(g_bufferText);
    
    // 步驟6：清除暫放區內容
    wstring sentText = g_bufferText; // 保存已發送的文字用於顯示
    g_bufferText.clear();
    g_bufferCursorPos = 0;
    save_buffer_to_file();
    
 
    
    update_status(L"已發送：" + sentText.substr(0, min(10, (int)sentText.length())) + 
                 (sentText.length() > 10 ? L"..." : L""));
}

// ========== 視窗程序 ==========
LRESULT CALLBACK BufferWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE:
            g_bufferCursorBlinkTime = GetTickCount();
            SetTimer(hwnd, 1, 500, NULL);
            break;
            
        case WM_TIMER:
            if (wParam == 1) {
                g_bufferShowCursor = !g_bufferShowCursor;
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
            
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            draw_buffer_window(hdc, clientRect);
            EndPaint(hwnd, &ps);
            break;
        }
            
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // 修復問題4：「發送」按鈕點擊處理
            if (PtInRect(&g_sendButtonRect, POINT{x, y})) {
                send_buffer_content();  // 調用外部定義的函數
                return 0;
            }
            
            if (PtInRect(&g_clearButtonRect, POINT{x, y})) {
                clear_buffer_with_confirm();
                return 0;
            }
            
            if (PtInRect(&g_saveButtonRect, POINT{x, y})) {
                save_buffer_to_timestamped_file();
                return 0;
            }
            
            // 點擊文字區域處理光標位置
            if (y > BUFFER_CONTROL_BAR_HEIGHT) {
                set_cursor_position(x, y);
                g_bufferHasFocus = true;
                SetFocus(hwnd);
                return 0;
            }
            
            // 開始拖動 - 修復：使用正確的變數名稱
            g_isBufferDragging = true;
            GetCursorPos(&g_bufferDragStart);
            SetCapture(hwnd);
            return 0;
        }
        
        case WM_MOUSEMOVE: {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            
            // 修復：處理拖拽
            if (g_isBufferDragging) {
                POINT currentPos;
                GetCursorPos(&currentPos);
                int deltaX = currentPos.x - g_bufferDragStart.x;
                int deltaY = currentPos.y - g_bufferDragStart.y;
                
                RECT windowRect;
                GetWindowRect(hwnd, &windowRect);
                SetWindowPos(hwnd, NULL, windowRect.left + deltaX, windowRect.top + deltaY,
                           0, 0, SWP_NOSIZE | SWP_NOZORDER);
                
                g_bufferDragStart = currentPos;
                return 0;
            }
            
            // 按鈕懸停效果
            bool oldSendHover = g_sendButtonHover;
            bool oldClearHover = g_clearButtonHover;
            bool oldSaveHover = g_saveButtonHover;
            
            g_sendButtonHover = PtInRect(&g_sendButtonRect, POINT{x, y});
            g_clearButtonHover = PtInRect(&g_clearButtonRect, POINT{x, y});
            g_saveButtonHover = PtInRect(&g_saveButtonRect, POINT{x, y});
            
            if (oldSendHover != g_sendButtonHover || 
                oldClearHover != g_clearButtonHover || 
                oldSaveHover != g_saveButtonHover) {
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        
        case WM_LBUTTONUP:
            if (g_isBufferDragging) {
                g_isBufferDragging = false;
                ReleaseCapture();
            }
            break;
        
        case WM_SETFOCUS:
            g_bufferHasFocus = true;
            break;
            
        case WM_KILLFOCUS:
            g_bufferHasFocus = false;
            break;
            
        case WM_CLOSE:
            toggle_buffer_mode();
            return 0;
            
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            break;
            
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ========== 其他輔助函數 ==========
void draw_buffer_window(HDC hdc, RECT& clientRect) {
    // 背景
    HBRUSH bgBrush = CreateSolidBrush(g_bufferBackgroundColor);
    FillRect(hdc, &clientRect, bgBrush);
    DeleteObject(bgBrush);
    
    // 控制列
    RECT controlRect = {0, 0, clientRect.right, BUFFER_CONTROL_BAR_HEIGHT};
    HBRUSH controlBrush = CreateSolidBrush(RGB(240, 240, 240));
    FillRect(hdc, &controlRect, controlBrush);
    DeleteObject(controlBrush);
    
    // 按鈕
    int buttonWidth = 50, buttonHeight = 20, buttonY = 5;
    g_sendButtonRect = {5, buttonY, 55, buttonY + buttonHeight};
    g_clearButtonRect = {60, buttonY, 110, buttonY + buttonHeight};
    g_saveButtonRect = {115, buttonY, 165, buttonY + buttonHeight};
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(0, 0, 0));
    
    // 發送按鈕
    HBRUSH sendBrush = CreateSolidBrush(g_sendButtonHover ? g_bufferButtonHoverColor : g_bufferButtonColor);
    FillRect(hdc, &g_sendButtonRect, sendBrush);
    DeleteObject(sendBrush);
    DrawEdge(hdc, &g_sendButtonRect, EDGE_RAISED, BF_RECT);
    DrawText(hdc, L"發送", -1, &g_sendButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // 清除按鈕
    HBRUSH clearBrush = CreateSolidBrush(g_clearButtonHover ? g_bufferButtonHoverColor : g_bufferButtonColor);
    FillRect(hdc, &g_clearButtonRect, clearBrush);
    DeleteObject(clearBrush);
    DrawEdge(hdc, &g_clearButtonRect, EDGE_RAISED, BF_RECT);
    DrawText(hdc, L"清除", -1, &g_clearButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // 儲存按鈕
    HBRUSH saveBrush = CreateSolidBrush(g_saveButtonHover ? g_bufferButtonHoverColor : g_bufferButtonColor);
    FillRect(hdc, &g_saveButtonRect, saveBrush);
    DeleteObject(saveBrush);
    DrawEdge(hdc, &g_saveButtonRect, EDGE_RAISED, BF_RECT);
    DrawText(hdc, L"儲存", -1, &g_saveButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    
    // 文字內容
    RECT textRect = {10, BUFFER_CONTROL_BAR_HEIGHT + 5, clientRect.right - 10, clientRect.bottom - 5};
    SetTextColor(hdc, g_bufferTextColor);
    
    if (!g_bufferText.empty()) {
        DrawText(hdc, g_bufferText.c_str(), -1, &textRect, DT_LEFT | DT_TOP | DT_WORDBREAK);
    }
    
    // 游標
    if (g_bufferShowCursor && g_bufferHasFocus) {
        int cursorLine = g_bufferCursorPos / BUFFER_CHARS_PER_LINE;
        int cursorCol = g_bufferCursorPos % BUFFER_CHARS_PER_LINE;
        int cursorX = 10 + cursorCol * 16;
        int cursorY = BUFFER_CONTROL_BAR_HEIGHT + 5 + cursorLine * BUFFER_LINE_HEIGHT;
        
        HPEN cursorPen = CreatePen(PS_SOLID, 1, g_bufferCursorColor);
        HPEN oldPen = (HPEN)SelectObject(hdc, cursorPen);
        MoveToEx(hdc, cursorX, cursorY, NULL);
        LineTo(hdc, cursorX, cursorY + BUFFER_LINE_HEIGHT - 2);
        SelectObject(hdc, oldPen);
        DeleteObject(cursorPen);
    }
}

void draw_buffer_button(HDC hdc) {
    COLORREF bufferColor, textColor;
    if (g_bufferMode) {
        bufferColor = RGB(100, 149, 237);
        textColor = RGB(255, 255, 255);
    } else if (g_bufferButtonHover) {
        bufferColor = RGB(220, 220, 220);
        textColor = RGB(60, 60, 60);
    } else {
        bufferColor = RGB(240, 240, 240);
        textColor = RGB(100, 100, 100);
    }
    
    HBRUSH hBrush = CreateSolidBrush(bufferColor);
    FillRect(hdc, &g_bufferButtonRect, hBrush);
    DeleteObject(hBrush);
    
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor);
    DrawText(hdc, L"暫放", -1, &g_bufferButtonRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void save_buffer_to_file() {
    ofstream fout("textbuffer.txt");
    if (fout.is_open()) {
        fout << wstr_to_utf8(g_bufferText);
        fout.close();
    }
}

void load_buffer_from_file() {
    ifstream fin("textbuffer.txt");
    if (fin.is_open()) {
        string line, allText;
        while (getline(fin, line)) {
            allText += line + "\n";
        }
        if (!allText.empty() && allText.back() == '\n') {
            allText.pop_back();
        }
        g_bufferText = utf8_to_wstr(allText);
        fin.close();
        g_bufferCursorPos = min(g_bufferCursorPos, (int)g_bufferText.length());
    }
}

void save_buffer_to_timestamped_file() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char filename[256];
    sprintf(filename, "stroke_%04d%02d%02d_%02d%02d%02d.txt",
            1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday,
            ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    
    ofstream fout(filename);
    if (fout.is_open()) {
        fout << wstr_to_utf8(g_bufferText);
        fout.close();
        update_status(L"已儲存到 " + utf8_to_wstr(string(filename)));
    }
}

void clear_buffer_with_confirm() {
    if (MessageBox(g_hBufferWnd, L"確定要清除暫放區內容嗎？", L"確認", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        g_bufferText.clear();
        g_bufferCursorPos = 0;
        if (g_hBufferWnd) {
            InvalidateRect(g_hBufferWnd, NULL, TRUE);
        }
        update_status(L"暫放區已清除");
    }
}

void send_buffer_text_and_clear() {
    if (g_bufferText.empty()) return;
    
    send_text_direct_unicode(g_bufferText);
    g_bufferText.clear();
    g_bufferCursorPos = 0;
    InvalidateRect(g_hBufferWnd, NULL, TRUE);
    update_status(L"文字已發送並清除");
}

void insert_text_at_cursor(const wstring& text) {
    if (g_bufferCursorPos < 0) g_bufferCursorPos = 0;
    if (g_bufferCursorPos > (int)g_bufferText.length()) g_bufferCursorPos = g_bufferText.length();
    
    g_bufferText.insert(g_bufferCursorPos, text);
    g_bufferCursorPos += text.length();
    
    if (g_hBufferWnd) {
        int newHeight = calculate_buffer_window_height();
        RECT rect;
        GetWindowRect(g_hBufferWnd, &rect);
        SetWindowPos(g_hBufferWnd, NULL, 0, 0, BUFFER_FIXED_WIDTH, newHeight, 
                     SWP_NOMOVE | SWP_NOZORDER);
        InvalidateRect(g_hBufferWnd, NULL, TRUE);
    }
}

void delete_char_at_cursor(bool forward) {
    if (g_bufferText.empty()) return;
    
    if (forward) {
        if (g_bufferCursorPos < (int)g_bufferText.length()) {
            g_bufferText.erase(g_bufferCursorPos, 1);
        }
    } else {
        if (g_bufferCursorPos > 0) {
            g_bufferText.erase(g_bufferCursorPos - 1, 1);
            g_bufferCursorPos--;
        }
    }
    
    if (g_hBufferWnd) {
        InvalidateRect(g_hBufferWnd, NULL, TRUE);
    }
}

void move_cursor(int direction) {
    if (direction < 0 && g_bufferCursorPos > 0) {
        g_bufferCursorPos--;
    } else if (direction > 0 && g_bufferCursorPos < (int)g_bufferText.length()) {
        g_bufferCursorPos++;
    }
    
    if (g_hBufferWnd) {
        InvalidateRect(g_hBufferWnd, NULL, FALSE);
    }
}

void set_cursor_position(int x, int y) {
    int charIndex = ((y - BUFFER_CONTROL_BAR_HEIGHT) / BUFFER_LINE_HEIGHT) * BUFFER_CHARS_PER_LINE + (x / 16);
    g_bufferCursorPos = min(max(0, charIndex), (int)g_bufferText.length());
    
    if (g_hBufferWnd) {
        InvalidateRect(g_hBufferWnd, NULL, FALSE);
    }
}

bool should_intercept_for_buffer(int key) {
    if (!is_buffer_window_active()) return false;
    
    if ((key >= 'A' && key <= 'Z') || (key >= '0' && key <= '9')) return true;
    if (key >= VK_OEM_1 && key <= VK_OEM_3) return true;
    if (key >= VK_OEM_4 && key <= VK_OEM_7) return true;
    if (key == VK_SPACE || key == VK_RETURN) return true;
    if (key == VK_BACK || key == VK_DELETE) return true;
    if (key == VK_LEFT || key == VK_RIGHT || key == VK_HOME || key == VK_END) return true;
    
    return false;
}

void handle_buffer_key_input(int key, bool shift, bool ctrl) {
    if (!g_bufferMode || !g_hBufferWnd) return;
    
    // ========== 終極修復：筆劃鍵完全攔截 ==========
    // 如果是筆劃鍵，直接返回，不做任何處理
    if (key == 'U' || key == 'I' || key == 'O' || 
        key == 'J' || key == 'K' || key == 'L' ||
        key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 ||
        key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0) {
        // 筆劃鍵完全不處理，避免被當作英文字母
        return;
    }
    
    // Ctrl組合鍵
    if (ctrl) {
        switch (key) {
            case 'V': // 貼上
                if (OpenClipboard(g_hBufferWnd)) {
                    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
                    if (hData) {
                        wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
                        if (pszText) {
                            insert_text_at_cursor(wstring(pszText));
                        }
                        GlobalUnlock(hData);
                    }
                    CloseClipboard();
                }
                break;
            case 'A': // 全選
                g_bufferCursorPos = g_bufferText.length();
                break;
        }
        return;
    }
    
    switch (key) {
        case VK_RETURN: // Enter
            if (!g_bufferText.empty()) {
                send_buffer_content();
            }
            break;
        case VK_BACK:
            delete_char_at_cursor(false);
            break;
        case VK_DELETE:
            delete_char_at_cursor(true);
            break;
        case VK_LEFT:
            move_cursor(-1);
            break;
        case VK_RIGHT:
            move_cursor(1);
            break;
        case VK_HOME:
            g_bufferCursorPos = 0;
            if (g_hBufferWnd) InvalidateRect(g_hBufferWnd, NULL, FALSE);
            break;
        case VK_END:
            g_bufferCursorPos = g_bufferText.length();
            if (g_hBufferWnd) InvalidateRect(g_hBufferWnd, NULL, FALSE);
            break;
        case VK_SPACE:
            insert_text_at_cursor(L" ");
            break;
        default:
            if (key >= 'A' && key <= 'Z') {
                // 注意：筆劃鍵已在函數開頭被過濾，這裡不會處理到
                wchar_t ch = (wchar_t)key;
                if (!shift) ch = towlower(ch);
                insert_text_at_cursor(wstring(1, ch));
            } else if (key >= '0' && key <= '9') {
                if (shift) {
                    const wchar_t symbols[] = L")!@#$%^&*(";
                    insert_text_at_cursor(wstring(1, symbols[key - '0']));
                } else {
                    insert_text_at_cursor(wstring(1, (wchar_t)key));
                }
            }
            break;
    }
}

