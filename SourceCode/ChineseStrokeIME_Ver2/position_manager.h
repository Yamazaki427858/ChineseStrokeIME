// position_manager.h - 位置記憶管理（改進版）
#ifndef POSITION_MANAGER_H
#define POSITION_MANAGER_H

#include "ime_core.h"

namespace PositionManager {
    // 位置結構
    struct Position {
        int x, y;
        bool isValid;
        
        Position() : x(0), y(0), isValid(false) {}
        Position(int _x, int _y) : x(_x), y(_y), isValid(true) {}
    };
    
    // 螢幕模式位置結構
    struct ScreenModePositions {
        Position extendedModePos;    // 延伸模式位置
        Position mirroredModePos;    // 鏡像模式位置
        bool hasExtendedPos = false;
        bool hasMirroredPos = false;
    };
    
    // 外部可訪問變數
    extern Position g_toolbarPos;
    extern ScreenModePositions g_screenModePositions;
    extern bool g_useUserPosition;
    extern Position g_userInputPos;
    extern Position g_userCandPos;
    extern int g_verticalOffset;
    
    // 位置載入與儲存
    void loadPositions(GlobalState& state);
    void savePositions(const GlobalState& state);
    
    // 螢幕模式相關位置調整
    void adjustPositionForScreenMode(GlobalState& state);
    void ensureVisiblePosition(GlobalState& state);
    
    // 位置設定
    POINT getCurrentMousePosition();
    POINT getOptimalWindowPosition(const GlobalState& state);
    
    // 新增：位置驗證和安全重置功能
    bool isPositionVisible(const GlobalState& state);
    void forceResetToSafePosition(GlobalState& state);
}

#endif // POSITION_MANAGER_H