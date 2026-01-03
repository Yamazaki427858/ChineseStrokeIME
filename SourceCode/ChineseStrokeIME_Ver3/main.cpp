// main.cpp - 中文筆劃輸入法主程式
#include "ime_core.h"
#include "window_manager.h"
#include "input_handler.h"
#include "dictionary.h"
#include "buffer_manager.h"
#include "config_loader.h"
#include "screen_manager.h"
#include "position_manager.h"
#include "tray_manager.h"
#include "ime_manager.h"
#include <windows.h>

// 全域變數實例
GlobalState g_state;
HHOOK g_hKeyboardHook = NULL;
TrayManager::TrayIconData g_trayIcon;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    try {
        // 初始化輸入法管理器（避免與 Windows 輸入法衝突）
        IMEManager::initialize();
        
        // 初始化螢幕資訊
        ScreenManager::updateMonitorInfo();
        
        // 載入設定
        ConfigLoader::loadInterfaceConfig(g_state);
        Dictionary::loadMainDict("Zi-Ma-Biao.txt", g_state);
        Dictionary::loadPunctuator(g_state);
        Dictionary::loadPunctMenu(g_state);
        Dictionary::loadUserDict(g_state);
        Dictionary::loadWordPhrases(g_state);  // 載入詞語庫（可選）
        
        // 載入位置記憶
        PositionManager::loadPositions(g_state);
        
        // 設定為OptimizedUI模式
        g_state.useOptimizedUI = true;
        WindowManager::switchToOptimizedUI(g_state);
        
        // 註冊視窗類別
        if (!WindowManager::registerOptimizedWindowClasses(hInstance)) {
            MessageBoxW(NULL, L"無法註冊視窗類別", L"錯誤", MB_OK | MB_ICONERROR);
            return 1;
        }
        
        // 建立視窗
        if (!WindowManager::createOptimizedWindows(hInstance, g_state)) {
            MessageBoxW(NULL, L"無法建立視窗", L"錯誤", MB_OK | MB_ICONERROR);
            return 1;
        }
		
        // 字碼視窗初始化
        if (g_state.hInputWnd) {
            ShowWindow(g_state.hInputWnd, SW_HIDE);  // 初始隱藏
        }
		
        // 確保視窗位置在可見區域
        PositionManager::ensureVisiblePosition(g_state);
        
        // 安裝鍵盤鉤子
        g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, InputHandler::KeyboardHookProc, hInstance, 0);
        if (!g_hKeyboardHook) {
            MessageBoxW(NULL, L"無法安裝鍵盤鉤子，可能需要管理員權限", L"警告", MB_OK | MB_ICONWARNING);
        }
        
        // 建立系統托盤圖示
        TrayManager::createTrayIcon(g_state.hWnd, &g_trayIcon);
        
        // 顯示主視窗
        ShowWindow(g_state.hWnd, SW_SHOW);
        UpdateWindow(g_state.hWnd);
        
        // 應用透明度設置（如果已啟用）
        WindowManager::applyTransparency(g_state);
        
        // 初始化暫放視窗位置
        if (g_state.bufferMode) {
            WindowManager::updateBufferWindowPosition(g_state);
        }
        
        // 設定初始狀態訊息
        Utils::updateStatus(g_state, L"採用OptimizedChineseStrokeIME UI風格的中文筆劃輸入法已啟動");
        
        // 注意：不再使用 IME API，改用鍵盤鉤子直接轉換字母
        
        // 新增：延遲位置檢查和初始化驗證
        SetTimer(g_state.hWnd, 996, 100, NULL);
        
        // 啟動時自動檢查版本更新（延遲3秒，不阻塞啟動）
        SetTimer(g_state.hWnd, 994, 3000, NULL);
        
        // 訊息迴圈
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        // 程式結束前的清理工作
        if (g_hKeyboardHook) {
            UnhookWindowsHookEx(g_hKeyboardHook);
        }
        
        // 儲存用戶設定和學習記錄
        Dictionary::saveUserDict(g_state);
        PositionManager::savePositions(g_state);
        
        // 移除系統托盤圖示
        TrayManager::removeTrayIcon(&g_trayIcon);
        
        // 清理輸入法管理器
        IMEManager::cleanup();
        
        return (int)msg.wParam;
        
    } catch (...) {
        MessageBoxW(NULL, L"程式發生未預期的錯誤", L"錯誤", MB_OK | MB_ICONERROR);
        
        // 緊急清理
        if (g_hKeyboardHook) {
            UnhookWindowsHookEx(g_hKeyboardHook);
        }
        TrayManager::removeTrayIcon(&g_trayIcon);
        IMEManager::cleanup();
        
        return 1;
    }
}
