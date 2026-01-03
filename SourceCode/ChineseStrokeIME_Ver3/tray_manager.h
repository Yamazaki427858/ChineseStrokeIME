// tray_manager.h - 系統托盤管理
#ifndef TRAY_MANAGER_H
#define TRAY_MANAGER_H

#include "ime_core.h"
#include <shellapi.h>

namespace TrayManager {
    // 托盤圖示設定
    struct TrayIconData {
        NOTIFYICONDATA nid;
        bool isMinimized;
    };
    
    // 托盤圖示操作
    void createTrayIcon(HWND hwnd, TrayIconData* trayIcon);
    void removeTrayIcon(TrayIconData* trayIcon);
    void updateTrayIcon(TrayIconData* trayIcon, HICON hIcon);
    void updateTrayTooltip(TrayIconData* trayIcon, const std::wstring& tooltip);
    
    // 視窗與托盤交互
    void showFromTray(HWND hwnd, TrayIconData* trayIcon);
    void hideToTray(HWND hwnd, TrayIconData* trayIcon);
    
    // 托盤選單
    void showTrayMenu(HWND hwnd, GlobalState& state);
    void processTrayMessage(HWND hwnd, LPARAM lParam, GlobalState& state);
}

#endif // TRAY_MANAGER_H