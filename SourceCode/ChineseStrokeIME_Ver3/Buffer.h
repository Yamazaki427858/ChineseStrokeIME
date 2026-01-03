#pragma once
#ifndef BUFFER_H
#define BUFFER_H

#include <windows.h>
#include <string>

using namespace std;
struct Position {
    int x;
    int y;
    bool isValid;
};
// ========== 暫放模式全域變數聲明 ==========
extern bool g_bufferMode;                    // 暫放模式開關
extern HWND g_hBufferWnd;                    // 暫放視窗句柄
extern wstring g_bufferText;                 // 暫放區文字內容
extern int g_bufferCursorPos;                // 游標位置
extern bool g_bufferShowCursor;             // 游標顯示狀態
extern DWORD g_bufferCursorBlinkTime;       // 游標閃爍計時
extern bool g_bufferHasFocus;               // 暫放視窗焦點狀態

// 暫放視窗配置常數
extern const int BUFFER_FIXED_WIDTH;        // 暫放視窗固定寬度
extern const int BUFFER_CHARS_PER_LINE;     // 每行字符數
extern const int BUFFER_MIN_HEIGHT;         // 最小高度
extern const int BUFFER_MAX_HEIGHT;         // 最大高度
extern const int BUFFER_LINE_HEIGHT;        // 行高
extern const int BUFFER_CONTROL_BAR_HEIGHT; // 控制欄高度

// 🔥 不定義 Position 結構體，使用主程序中的定義

// 暫放視窗位置管理
extern struct Position g_bufferFixedOffset; // 使用主程序的 Position 類型
//extern bool g_isBufferDragging;             // 拖拽狀態
//extern POINT g_bufferDragStart;             // 拖拽起點
//extern POINT g_bufferDragOffset;            // 拖拽偏移

// UI 元素矩形區域
extern RECT g_sendButtonRect;
extern RECT g_clearButtonRect;
extern RECT g_saveButtonRect;
extern bool g_sendButtonHover;
extern bool g_clearButtonHover;
extern bool g_saveButtonHover;
extern RECT g_bufferButtonRect;              // 工具列暫放按鈕

// 暫放視窗顏色配置
extern COLORREF g_bufferBackgroundColor;
extern COLORREF g_bufferTextColor;
extern COLORREF g_bufferCursorColor;
extern COLORREF g_bufferButtonColor;
extern COLORREF g_bufferButtonHoverColor;

// ========== 暫放模式函數聲明 ==========

// 核心管理函數
void toggle_buffer_mode();
void init_buffer_window();
void cleanup_buffer_window();

// 視窗處理函數
LRESULT CALLBACK BufferWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 文字操作函數
void insert_text_at_cursor(const wstring& text);
void delete_char_at_cursor(bool forward = true);
void move_cursor(int direction);
void set_cursor_position(int x, int y);

// 視窗繪製函數
void draw_buffer_window(HDC hdc, RECT& clientRect);
void draw_buffer_button(HDC hdc);
void update_buffer_position();

// 工具函數
int calculate_buffer_window_height();
bool is_buffer_window_active();
void position_buffer_window();

// 檔案操作函數
void save_buffer_to_file();
void load_buffer_from_file();
void save_buffer_to_timestamped_file();

// 內容操作函數
void send_buffer_content();
void clear_buffer_with_confirm();
void send_buffer_text_and_clear();

// 鍵盤處理函數
bool should_intercept_for_buffer(int key);
void handle_buffer_key_input(int key, bool shift, bool ctrl);

// UTF-8 轉換函數
wstring utf8_to_wstr(const string& str);
string wstr_to_utf8(const wstring& ws);

void send_text_direct_unicode(const wstring& text);



#endif // BUFFER_H
