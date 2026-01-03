// screen_manager.cpp - 螢幕偵測管理實作（快速修復版）
#include "screen_manager.h"
#include <algorithm>

namespace ScreenManager {

std::vector<MonitorInfo> g_monitors;

// 新增：狀態追蹤變數
static bool g_previousExtendedMode = false;
static bool g_firstTimeCheck = true;

// 螢幕枚舉回呼函數
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

void updateMonitorInfo() {
    g_monitors.clear();
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);
    
    // 如果找不到螢幕，確保至少有一個預設螢幕
    if (g_monitors.empty()) {
        MonitorInfo defaultMonitor;
        defaultMonitor.rect.left = 0;
        defaultMonitor.rect.top = 0;
        defaultMonitor.rect.right = GetSystemMetrics(SM_CXSCREEN);
        defaultMonitor.rect.bottom = GetSystemMetrics(SM_CYSCREEN);
        defaultMonitor.workArea = defaultMonitor.rect;
        defaultMonitor.isPrimary = true;
        defaultMonitor.hMonitor = NULL;
        g_monitors.push_back(defaultMonitor);
    }
}

std::vector<MonitorInfo> getMonitors() {
    if (g_monitors.empty()) {
        updateMonitorInfo();
    }
    return g_monitors;
}

MonitorInfo getMonitorFromPoint(POINT pt) {
    if (g_monitors.empty()) {
        updateMonitorInfo();
    }
    
    HMONITOR hMon = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    
    for (const auto& monitor : g_monitors) {
        if (monitor.hMonitor == hMon) {
            return monitor;
        }
    }
    
    // 回退到主螢幕
    return getPrimaryMonitor();
}

MonitorInfo getPrimaryMonitor() {
    if (g_monitors.empty()) {
        updateMonitorInfo();
    }
    
    for (const auto& monitor : g_monitors) {
        if (monitor.isPrimary) {
            return monitor;
        }
    }
    
    // 如果找不到主螢幕，返回第一個
    return g_monitors[0];
}

bool isPointInAnyMonitor(POINT pt) {
    if (g_monitors.empty()) {
        updateMonitorInfo();
    }
    
    for (const auto& monitor : g_monitors) {
        if (pt.x >= monitor.rect.left && pt.x <= monitor.rect.right &&
            pt.y >= monitor.rect.top && pt.y <= monitor.rect.bottom) {
            return true;
        }
    }
    return false;
}

bool isExtendedMode() {
    updateMonitorInfo();
    return g_monitors.size() > 1;
}

// 新增：更精確的鏡像模式檢測
bool isTrulyMirroredMode() {
    updateMonitorInfo();
    
    if (g_monitors.size() <= 1) return true;
    
    // 檢查所有螢幕是否有相同的解析度（鏡像模式特徵）
    if (g_monitors.size() >= 2) {
        RECT firstRect = g_monitors[0].rect;
        int firstWidth = firstRect.right - firstRect.left;
        int firstHeight = firstRect.bottom - firstRect.top;
        
        for (size_t i = 1; i < g_monitors.size(); i++) {
            RECT currentRect = g_monitors[i].rect;
            int currentWidth = currentRect.right - currentRect.left;
            int currentHeight = currentRect.bottom - currentRect.top;
            
            if (currentWidth != firstWidth || currentHeight != firstHeight) {
                return false; // 不同解析度，確實是延伸模式
            }
        }
        return true; // 相同解析度，可能是鏡像模式
    }
    
    return false;
}

// 替換：改用新的檢測邏輯
bool isMirroredMode() {
    return isTrulyMirroredMode();
}

bool isCoordinateValidInCurrentMode(int x, int y) {
    updateMonitorInfo();
    
    for (const auto& monitor : g_monitors) {
        if (x >= monitor.workArea.left && x <= monitor.workArea.right &&
            y >= monitor.workArea.top && y <= monitor.workArea.bottom) {
            return true;
        }
    }
    return false;
}

// 快速修復：簡化的顯示變更處理
void handleDisplayChange() {
    updateMonitorInfo();
    
    // 強制重新檢測模式
    bool currentExtended = isExtendedMode();
    
    if (!g_firstTimeCheck) {
        bool modeChanged = (g_previousExtendedMode != currentExtended);
        
        if (modeChanged) {
            // 暫時註解掉，避免編譯錯誤
            /*
            extern GlobalState g_state;
            if (g_state.hWnd) {
                PostMessage(g_state.hWnd, WM_USER + 301, currentExtended ? 1 : 0, 0);
            }
            */
        }
    } else {
        g_firstTimeCheck = false;
    }
    
    g_previousExtendedMode = currentExtended;
}

RECT getSafePrimaryScreen() {
    RECT safeRect = {0, 0, 1920, 1080}; // 預設安全值
    
    updateMonitorInfo();
    
    if (g_monitors.size() <= 1) {
        // 鏡像模式：使用系統工作區域
        if (SystemParametersInfo(SPI_GETWORKAREA, 0, &safeRect, 0)) {
            return safeRect;
        }
    } else {
        // 延伸模式：使用主螢幕
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

POINT ensureSafePosition(POINT pt, int windowWidth, int windowHeight) {
    if (g_monitors.empty()) {
        updateMonitorInfo();
    }
    
    // 檢查點是否在任何螢幕上
    bool pointInScreen = false;
    for (const auto& monitor : g_monitors) {
        if (pt.x >= monitor.workArea.left && 
            pt.x + windowWidth <= monitor.workArea.right &&
            pt.y >= monitor.workArea.top &&
            pt.y + windowHeight <= monitor.workArea.bottom) {
            pointInScreen = true;
            break;
        }
    }
    
    if (!pointInScreen) {
        // 如果點不在任何螢幕內，使用主螢幕的中央
        MonitorInfo primary = getPrimaryMonitor();
        pt.x = primary.workArea.left + (primary.workArea.right - primary.workArea.left - windowWidth) / 2;
        pt.y = primary.workArea.top + (primary.workArea.bottom - primary.workArea.top - windowHeight) / 2;
    }
    
    return pt;
}

} // namespace ScreenManager