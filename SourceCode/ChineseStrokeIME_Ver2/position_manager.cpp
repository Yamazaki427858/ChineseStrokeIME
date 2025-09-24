// position_manager.cpp - 位置記憶管理實作（改進版）
#include "position_manager.h"
#include "screen_manager.h"
#include <fstream>

namespace PositionManager {

// 全域變數
Position g_toolbarPos;
ScreenModePositions g_screenModePositions;
bool g_useUserPosition = false;
Position g_userInputPos;
Position g_userCandPos;

// 垂直偏移量
int g_verticalOffset = 25;

// 新增：重試計數器
static int g_retryCount = 0;

POINT getCurrentMousePosition() {
    POINT pos;
    GetCursorPos(&pos);
    
    // 檢查滑鼠是否在任何螢幕範圍內
    bool inAnyScreen = false;
    auto monitors = ScreenManager::getMonitors();
    for (const auto& monitor : monitors) {
        if (pos.x >= monitor.rect.left && pos.x <= monitor.rect.right &&
            pos.y >= monitor.rect.top && pos.y <= monitor.rect.bottom) {
            inAnyScreen = true;
            break;
        }
    }
    
    if (!inAnyScreen) {
        // 如果不在任何螢幕內，使用主螢幕中央
        ScreenManager::MonitorInfo primary = ScreenManager::getPrimaryMonitor();
        pos.x = primary.rect.left + (primary.rect.right - primary.rect.left) / 2;
        pos.y = primary.rect.top + (primary.rect.bottom - primary.rect.top) / 2;
    }
    
    pos.y += g_verticalOffset;
    return pos;
}

POINT getOptimalWindowPosition(const GlobalState& state) {
    if (g_useUserPosition && g_userInputPos.isValid) {
        POINT result = {g_userInputPos.x, g_userInputPos.y};
        return result;
    }
    
    POINT basePos = getCurrentMousePosition();
    ScreenManager::MonitorInfo currentMonitor = ScreenManager::getMonitorFromPoint(basePos);
    
    int totalHeight = state.windowHeight;
    if (state.showCand) {
        totalHeight += 5 + state.candidateHeight;
    }
    
    // 確保視窗在螢幕可見範圍內
    if (basePos.x + state.windowWidth > currentMonitor.workArea.right) {
        basePos.x = currentMonitor.workArea.right - state.windowWidth - 10;
    }
    if (basePos.x < currentMonitor.workArea.left) {
        basePos.x = currentMonitor.workArea.left + 10;
    }
    
    if (basePos.y + totalHeight > currentMonitor.workArea.bottom) {
        basePos.y = basePos.y - totalHeight - g_verticalOffset;
        if (basePos.y < currentMonitor.workArea.top) {
            basePos.y = currentMonitor.workArea.top + 10;
        }
    }
    
    return basePos;
}

// 新增：檢查位置是否可見
bool isPositionVisible(const GlobalState& state) {
    if (!state.hWnd) return false;
    
    RECT windowRect;
    if (!GetWindowRect(state.hWnd, &windowRect)) return false;
    
    // 檢查視窗是否在任何螢幕內
    std::vector<ScreenManager::MonitorInfo> monitors = ScreenManager::getMonitors();
    for (const auto& monitor : monitors) {
        RECT intersection;
        if (IntersectRect(&intersection, &windowRect, &monitor.workArea)) {
            // 確保至少50%的視窗在螢幕內
            int windowArea = (windowRect.right - windowRect.left) * 
                           (windowRect.bottom - windowRect.top);
            int intersectionArea = (intersection.right - intersection.left) * 
                                 (intersection.bottom - intersection.top);
            if (intersectionArea >= windowArea / 2) {
                return true;
            }
        }
    }
    return false;
}

// 新增：強制重置到安全位置
void forceResetToSafePosition(GlobalState& state) {
    RECT safeScreen = ScreenManager::getSafePrimaryScreen();
    g_toolbarPos.x = safeScreen.left + 50;
    g_toolbarPos.y = safeScreen.bottom - state.windowHeight - 80;
    g_toolbarPos.isValid = true;
    
    if (state.hWnd) {
        SetWindowPos(state.hWnd, HWND_TOPMOST,
            g_toolbarPos.x, g_toolbarPos.y,
            0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
        
        ShowWindow(state.hWnd, SW_SHOW);
        SetForegroundWindow(state.hWnd);
    }
    
    savePositions(state);
    Utils::updateStatus(state, L"工具列已重置到安全位置");
}

void loadPositions(GlobalState& state) {
    std::ifstream config("positions.ini");
    if (!config.is_open()) {
        // 使用安全的螢幕偵測
        RECT primaryScreen = ScreenManager::getSafePrimaryScreen();
        
        g_toolbarPos.x = std::max<int>(primaryScreen.left + 50, 50);
        g_toolbarPos.y = std::max<int>(primaryScreen.bottom - state.windowHeight - 80, 50);
        g_toolbarPos.isValid = true;
        
        savePositions(state);
        return;
    }
    
    std::string line, section;
    while (std::getline(config, line)) {
        if (line.empty() || line[0] == '#') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            section = line.substr(1, line.length() - 2);
            continue;
        }
        
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        
        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);
        
        try {
            if (section == "Toolbar") {
                if (key == "x") g_toolbarPos.x = std::stoi(value);
                else if (key == "y") g_toolbarPos.y = std::stoi(value);
                g_toolbarPos.isValid = true;
            } else if (section == "UserPosition") {
                if (key == "enabled") g_useUserPosition = (value == "1");
                else if (key == "input_x") g_userInputPos.x = std::stoi(value);
                else if (key == "input_y") g_userInputPos.y = std::stoi(value);
                else if (key == "cand_x") g_userCandPos.x = std::stoi(value);
                else if (key == "cand_y") g_userCandPos.y = std::stoi(value);
                
                if (g_useUserPosition) {
                    g_userInputPos.isValid = true;
                    g_userCandPos.isValid = true;
                }
            } else if (section == "OptimizedPositioning") {
                if (key == "vertical_offset") g_verticalOffset = std::stoi(value);
            } else if (section == "ToolbarExtended") {
                if (key == "x") g_screenModePositions.extendedModePos.x = std::stoi(value);
                else if (key == "y") g_screenModePositions.extendedModePos.y = std::stoi(value);
                g_screenModePositions.extendedModePos.isValid = true;
                g_screenModePositions.hasExtendedPos = true;
            } else if (section == "ToolbarMirrored") {
                if (key == "x") g_screenModePositions.mirroredModePos.x = std::stoi(value);
                else if (key == "y") g_screenModePositions.mirroredModePos.y = std::stoi(value);
                g_screenModePositions.mirroredModePos.isValid = true;
                g_screenModePositions.hasMirroredPos = true;
            }
        } catch (...) {}
    }
    
    config.close();
    
    // 根據當前螢幕模式自動選擇適合的位置
    if (ScreenManager::isExtendedMode() && g_screenModePositions.hasExtendedPos) {
        g_toolbarPos = g_screenModePositions.extendedModePos;
    } else if (ScreenManager::isMirroredMode() && g_screenModePositions.hasMirroredPos) {
        g_toolbarPos = g_screenModePositions.mirroredModePos;
    }
    
    ensureVisiblePosition(state);
}

void savePositions(const GlobalState& state) {
    std::ofstream config("positions.ini");
    if (!config.is_open()) return;
    
    config << "[Toolbar]" << std::endl;
    config << "x=" << g_toolbarPos.x << std::endl;
    config << "y=" << g_toolbarPos.y << std::endl;
    
    // 根據當前螢幕模式儲存到對應區段
    if (ScreenManager::isExtendedMode()) {
        config << "[ToolbarExtended]" << std::endl;
        config << "x=" << g_toolbarPos.x << std::endl;
        config << "y=" << g_toolbarPos.y << std::endl;
        
        // 更新記憶中的位置
        g_screenModePositions.extendedModePos = g_toolbarPos;
        g_screenModePositions.hasExtendedPos = true;
    } else {
        config << "[ToolbarMirrored]" << std::endl;
        config << "x=" << g_toolbarPos.x << std::endl;
        config << "y=" << g_toolbarPos.y << std::endl;
        
        // 更新記憶中的位置
        g_screenModePositions.mirroredModePos = g_toolbarPos;
        g_screenModePositions.hasMirroredPos = true;
    }
    
    config << "[UserPosition]" << std::endl;
    config << "enabled=" << (g_useUserPosition ? "1" : "0") << std::endl;
    if (g_useUserPosition) {
        config << "input_x=" << g_userInputPos.x << std::endl;
        config << "input_y=" << g_userInputPos.y << std::endl;
        config << "cand_x=" << g_userCandPos.x << std::endl;
        config << "cand_y=" << g_userCandPos.y << std::endl;
    }
    
    config << "[OptimizedPositioning]" << std::endl;
    config << "vertical_offset=" << g_verticalOffset << std::endl;
    
    config.close();
}

// 改進：增強的螢幕模式切換處理
void adjustPositionForScreenMode(GlobalState& state) {
    static bool previousExtended = ScreenManager::isExtendedMode();
    static bool firstTime = true;
    
    if (firstTime) {
        previousExtended = ScreenManager::isExtendedMode();
        firstTime = false;
    }
    
    // 強制更新螢幕信息
    ScreenManager::updateMonitorInfo();
    
    bool currentExtended = ScreenManager::isExtendedMode();
    bool modeChanged = (previousExtended != currentExtended);
    
    if (modeChanged || g_retryCount > 0) {
        RECT safeScreen = ScreenManager::getSafePrimaryScreen();
        
        // 從延伸切換到鏡像模式
        if (previousExtended && !currentExtended) {
            // 立即強制移動到主螢幕安全位置
            g_toolbarPos.x = safeScreen.left + 50;
            g_toolbarPos.y = safeScreen.bottom - state.windowHeight - 80;
            
            // 立即更新工具列位置
            if (state.hWnd) {
                SetWindowPos(state.hWnd, HWND_TOPMOST, 
                    g_toolbarPos.x, g_toolbarPos.y, 
                    0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                
                // 強制顯示
                ShowWindow(state.hWnd, SW_SHOW);
                InvalidateRect(state.hWnd, nullptr, TRUE);
                UpdateWindow(state.hWnd);
            }
            
            // 重置重試計數
            g_retryCount = 0;
            
            // 顯示提示
            if (modeChanged) {
                MessageBoxW(NULL, 
                    L"偵測到螢幕模式變更：延伸→同步\n"
                    L"工具列已自動移至主螢幕安全位置。", 
                    L"螢幕模式變更", MB_OK | MB_ICONINFORMATION);
            }
        } 
        // 從鏡像切換到延伸模式
        else if (!previousExtended && currentExtended) {
            if (g_screenModePositions.hasExtendedPos) {
                g_toolbarPos = g_screenModePositions.extendedModePos;
                ensureVisiblePosition(state);
            }
            g_retryCount = 0;
        }
        
        savePositions(state);
        previousExtended = currentExtended;
    }
    
    // 驗證當前位置是否可見
    if (!isPositionVisible(state)) {
        g_retryCount++;
        if (g_retryCount <= 3) {
            // 延遲重試
            SetTimer(state.hWnd, 998, 200, NULL);
        } else {
            // 強制重置到安全位置
            forceResetToSafePosition(state);
            g_retryCount = 0;
        }
    } else {
        // 位置正常，重置重試計數
        g_retryCount = 0;
    }
}

void ensureVisiblePosition(GlobalState& state) {
    bool positionValid = false;
    std::vector<ScreenManager::MonitorInfo> monitors = ScreenManager::getMonitors();
    
    for (const auto& monitor : monitors) {
        if (g_toolbarPos.x >= monitor.workArea.left && 
            g_toolbarPos.x <= monitor.workArea.right - state.windowWidth &&
            g_toolbarPos.y >= monitor.workArea.top &&
            g_toolbarPos.y <= monitor.workArea.bottom - state.windowHeight) {
            positionValid = true;
            break;
        }
    }
    
    if (!positionValid) {
        RECT safeScreen = ScreenManager::getSafePrimaryScreen();
        g_toolbarPos.x = safeScreen.left + 50;
        g_toolbarPos.y = safeScreen.bottom - state.windowHeight - 80;
        g_toolbarPos.isValid = true;
        
        // 更新視窗位置
        if (state.hWnd) {
            SetWindowPos(state.hWnd, NULL, g_toolbarPos.x, g_toolbarPos.y, 
                        0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        
        savePositions(state);
    }
}

} // namespace PositionManager