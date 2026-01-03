// ime_manager.cpp - Windows 輸入法衝突管理實作
#include "ime_manager.h"
#include <imm.h>

namespace IMEManager {
    
    // 保存原始輸入法狀態
    static HKL g_originalKeyboardLayout = NULL;
    static DWORD g_originalConversionMode = 0;
    static DWORD g_originalSentenceMode = 0;
    static HIMC g_savedIMC = NULL;
    static bool g_imeDisabled = false;
    static bool g_initialized = false;
    
    void initialize() {
        if (g_initialized) return;
        
        // 獲取當前鍵盤布局
        g_originalKeyboardLayout = GetKeyboardLayout(0);
        g_initialized = true;
    }
    
    bool isWindowsIMEActive() {
        HWND hForeground = GetForegroundWindow();
        if (!hForeground) return false;
        
        HIMC hIMC = ImmGetContext(hForeground);
        if (!hIMC) {
            // 如果沒有 IME 上下文，檢查鍵盤布局
            HKL currentLayout = GetKeyboardLayout(0);
            // 檢查是否為中文輸入法布局（繁體中文、簡體中文、日文、韓文等）
            LANGID langId = LOWORD(currentLayout);
            if (langId == 0x0404 || // 繁體中文
                langId == 0x0804 || // 簡體中文
                langId == 0x0411 || // 日文
                langId == 0x0412) { // 韓文
                return true;
            }
            return false;
        }
        
        DWORD conversionMode = 0;
        DWORD sentenceMode = 0;
        ImmGetConversionStatus(hIMC, &conversionMode, &sentenceMode);
        ImmReleaseContext(hForeground, hIMC);
        
        // 檢查是否處於中文輸入模式（IME 開啟狀態）
        // IME_CMODE_NATIVE 表示處於本地語言輸入模式（如中文、日文等）
        return (conversionMode & IME_CMODE_NATIVE) != 0;
    }
    
    void disableWindowsIME(bool force) {
        initialize();
        
        HWND hForeground = GetForegroundWindow();
        if (!hForeground) {
            // 嘗試對所有可見窗口禁用
            EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
                if (IsWindowVisible(hwnd)) {
                    HIMC hIMC = ImmGetContext(hwnd);
                    if (hIMC) {
                        DWORD conv, sent;
                        ImmGetConversionStatus(hIMC, &conv, &sent);
                        conv &= ~IME_CMODE_NATIVE;
                        conv |= IME_CMODE_ALPHANUMERIC;
                        ImmSetConversionStatus(hIMC, conv, sent);
                        ImmReleaseContext(hwnd, hIMC);
                    }
                }
                return TRUE;
            }, 0);
            return;
        }
        
        // 保存當前鍵盤布局（如果還沒保存）
        if (!g_originalKeyboardLayout) {
            g_originalKeyboardLayout = GetKeyboardLayout(0);
        }
        
        // 🔥 使用多種方法強制禁用 IME
        
        // 方法1：對前景窗口禁用
        HIMC hIMC = ImmGetContext(hForeground);
        if (hIMC) {
            // 保存當前狀態（只在第一次保存）
            if (!g_imeDisabled) {
                ImmGetConversionStatus(hIMC, &g_originalConversionMode, &g_originalSentenceMode);
                g_savedIMC = hIMC;
            }
            
            // 強制設置為英文模式
            DWORD newConversionMode = IME_CMODE_ALPHANUMERIC;
            newConversionMode &= ~IME_CMODE_NATIVE;
            newConversionMode &= ~IME_CMODE_FULLSHAPE;
            newConversionMode &= ~IME_CMODE_KATAKANA;
            // IME_CMODE_HIRAGANA 在某些編譯環境中可能未定義，使用條件編譯
            #ifdef IME_CMODE_HIRAGANA
            newConversionMode &= ~IME_CMODE_HIRAGANA;
            #endif
            
            ImmSetConversionStatus(hIMC, newConversionMode, g_originalSentenceMode);
            
            // 發送多個通知確保生效
            PostMessage(hForeground, WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0);
            SendMessage(hForeground, WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0);
            
            ImmReleaseContext(hForeground, hIMC);
        }
        
        // 方法2：對當前線程的所有窗口禁用
        DWORD currentThread = GetCurrentThreadId();
        EnumThreadWindows(currentThread, [](HWND hwnd, LPARAM lParam) -> BOOL {
            HIMC hIMC = ImmGetContext(hwnd);
            if (hIMC) {
                DWORD conv = IME_CMODE_ALPHANUMERIC;
                conv &= ~IME_CMODE_NATIVE;
                ImmSetConversionStatus(hIMC, conv, 0);
                ImmReleaseContext(hwnd, hIMC);
            }
            return TRUE;
        }, 0);
        
        // 方法3：對目標窗口的線程也禁用
        DWORD targetThread = GetWindowThreadProcessId(hForeground, NULL);
        if (targetThread != currentThread) {
            EnumThreadWindows(targetThread, [](HWND hwnd, LPARAM lParam) -> BOOL {
                HIMC hIMC = ImmGetContext(hwnd);
                if (hIMC) {
                    DWORD conv = IME_CMODE_ALPHANUMERIC;
                    conv &= ~IME_CMODE_NATIVE;
                    ImmSetConversionStatus(hIMC, conv, 0);
                    PostMessage(hwnd, WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0);
                    ImmReleaseContext(hwnd, hIMC);
                }
                return TRUE;
            }, 0);
        }
        
        g_imeDisabled = true;
    }
    
    void restoreWindowsIME() {
        if (!g_imeDisabled) return;
        
        HWND hForeground = GetForegroundWindow();
        if (!hForeground) return;
        
        // 🔥 改進：只恢復 IME 轉換模式，不切換鍵盤布局
        // 因為我們沒有切換鍵盤布局，所以也不需要恢復
        
        HIMC hIMC = ImmGetContext(hForeground);
        if (hIMC) {
            // 恢復原始 IME 轉換狀態
            ImmSetConversionStatus(hIMC, g_originalConversionMode, g_originalSentenceMode);
            
            // 發送 IME 狀態變更通知
            PostMessage(hForeground, WM_IME_NOTIFY, IMN_SETCONVERSIONMODE, 0);
            
            ImmReleaseContext(hForeground, hIMC);
        } else {
            // 嘗試恢復系統 IME 狀態
            hIMC = ImmGetContext(GetDesktopWindow());
            if (hIMC) {
                ImmSetConversionStatus(hIMC, g_originalConversionMode, g_originalSentenceMode);
                ImmReleaseContext(GetDesktopWindow(), hIMC);
            }
        }
        
        g_imeDisabled = false;
    }
    
    void cleanup() {
        if (g_imeDisabled) {
            restoreWindowsIME();
        }
        g_initialized = false;
    }
}

