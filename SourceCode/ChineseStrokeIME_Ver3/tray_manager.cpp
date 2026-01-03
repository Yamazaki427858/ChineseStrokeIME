// tray_manager.cpp - 系統托盤管理實作
#include "tray_manager.h"
#include "buffer_manager.h"
#include "config_loader.h"
#include "input_handler.h"
#include "position_manager.h"
#include "screen_manager.h"
#include "dictionary.h"

// 前向宣告
extern TrayManager::TrayIconData g_trayIcon;

namespace TrayManager {

void createTrayIcon(HWND hwnd, TrayIconData* trayIcon) {
    ZeroMemory(&trayIcon->nid, sizeof(NOTIFYICONDATA));
    trayIcon->nid.cbSize = sizeof(NOTIFYICONDATA);
    trayIcon->nid.hWnd = hwnd;
    trayIcon->nid.uID = 1;
    trayIcon->nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    trayIcon->nid.uCallbackMessage = WM_USER + 200;
    trayIcon->nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(trayIcon->nid.szTip, L"中文筆劃輸入法");
    trayIcon->isMinimized = false;
    Shell_NotifyIcon(NIM_ADD, &trayIcon->nid);
}

void removeTrayIcon(TrayIconData* trayIcon) {
    Shell_NotifyIcon(NIM_DELETE, &trayIcon->nid);
}

void updateTrayIcon(TrayIconData* trayIcon, HICON hIcon) {
    if (!trayIcon) return;
    
    trayIcon->nid.hIcon = hIcon;
    trayIcon->nid.uFlags = NIF_ICON;
    Shell_NotifyIcon(NIM_MODIFY, &trayIcon->nid);
}

void updateTrayTooltip(TrayIconData* trayIcon, const std::wstring& tooltip) {
    if (!trayIcon) return;
    
    wcsncpy_s(trayIcon->nid.szTip, tooltip.c_str(), 63);
    trayIcon->nid.szTip[63] = 0;
    trayIcon->nid.uFlags = NIF_TIP;
    Shell_NotifyIcon(NIM_MODIFY, &trayIcon->nid);
}

void showFromTray(HWND hwnd, TrayIconData* trayIcon) {
    trayIcon->isMinimized = false;
    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
}

void hideToTray(HWND hwnd, TrayIconData* trayIcon) {
    trayIcon->isMinimized = true;
    ShowWindow(hwnd, SW_HIDE);
}

void showTrayMenu(HWND hwnd, GlobalState& state) {
    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;
    
    // 基本功能
    if (g_trayIcon.isMinimized) {
        AppendMenu(hMenu, MF_STRING, 2001, L"📝 顯示輸入法");
    } else {
        AppendMenu(hMenu, MF_STRING, 2001, L"📌 顯示/置前");
    }
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    
    // 模式切換
    std::wstring modeText = state.chineseMode ? L"🔄 切換到英文模式" : L"🔄 切換到中文模式";
    AppendMenu(hMenu, MF_STRING, 2004, modeText.c_str());
    
    // 快捷功能
    AppendMenu(hMenu, MF_STRING, 2005, L"🔣 標點符號選單");
    
    if (state.bufferMode) {
        AppendMenu(hMenu, MF_STRING, 2010, L"📤 發送暫放文字");
        AppendMenu(hMenu, MF_STRING, 2011, L"✂️ 清空暫放區");
    } else {
        AppendMenu(hMenu, MF_STRING, 2012, L"📥 開啟暫放模式");
    }
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    
    // 輸入位置控制
    AppendMenu(hMenu, MF_STRING, 2002, L"🔄 重置為滑鼠跟隨");
    if (PositionManager::g_useUserPosition) {
        AppendMenu(hMenu, MF_STRING, 2007, L"🎯 取消固定位置");
    }
    
    AppendMenu(hMenu, MF_STRING, 2006, L"🔄 重新載入配置");
    AppendMenu(hMenu, MF_STRING, 2013, L"⬇️ 從GitHub更新字碼表");
    
    AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
    
    // 系統功能
    AppendMenu(hMenu, MF_STRING, 2008, L"ℹ️ 關於");
    AppendMenu(hMenu, MF_STRING, 2009, L"🔄 重啟輸入法");
    AppendMenu(hMenu, MF_STRING, 2003, L"❌ 關閉輸入法");
    
    // 獲取鼠標位置
    POINT pt;
    GetCursorPos(&pt);
    
    // 確保選單在頂層顯示，不被視窗遮蓋
    // 設置標誌，防止計時器在選單顯示時重新設置TOPMOST
    state.menuShowing = true;
    
    // 臨時移除窗口的TOPMOST屬性（如果有的話），避免與輸入法頂層功能衝突
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    bool wasTopmost = (exStyle & WS_EX_TOPMOST) != 0;
    if (wasTopmost) {
        SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    
    // 關鍵修復：確保窗口成為前景窗口
    SetForegroundWindow(hwnd);
    
    // 使用TrackPopupMenuEx來更好地控制選單位置和顯示
    TPMPARAMS tpmParams = {0};
    tpmParams.cbSize = sizeof(TPMPARAMS);
    tpmParams.rcExclude.left = pt.x - 1;
    tpmParams.rcExclude.top = pt.y - 1;
    tpmParams.rcExclude.right = pt.x + 1;
    tpmParams.rcExclude.bottom = pt.y + 1;
    
    // 顯示選單並等待用戶選擇
    int cmd = TrackPopupMenuEx(hMenu, 
                            TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_VERTICAL,
                            pt.x, pt.y, hwnd, &tpmParams);
    
    // 清除選單顯示標誌
    state.menuShowing = false;
    
    // 選單關閉後恢復窗口的TOPMOST狀態（如果原來是TOPMOST）
    if (wasTopmost) {
        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
    
    // 發送消息確保焦點正確
    PostMessage(hwnd, WM_NULL, 0, 0);
    
    // 清理選單
    DestroyMenu(hMenu);
    
    // 手動發送命令消息
    if (cmd > 0) {
        PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(cmd, 0), 0);
    }
    
    // 確保選單正確關閉
    PostMessage(hwnd, WM_NULL, 0, 0);
}

void processTrayMessage(HWND hwnd, LPARAM lParam, GlobalState& state) {
    switch (lParam) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
            if (g_trayIcon.isMinimized) {
                showFromTray(hwnd, &g_trayIcon);
            } else {
                SetForegroundWindow(hwnd);
                SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0,
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
            }
            break;
            
        case WM_RBUTTONUP:
            showTrayMenu(hwnd, state);
            break;
            
        case WM_RBUTTONDOWN:  // 也處理右鍵按下，防止遺漏
            // 不做任何動作，等待 WM_RBUTTONUP
            break;
    }
}

// 重啟輸入法函數
void restartIME() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(NULL, exePath, MAX_PATH);
    
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(STARTUPINFO);
    
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

} // namespace TrayManager