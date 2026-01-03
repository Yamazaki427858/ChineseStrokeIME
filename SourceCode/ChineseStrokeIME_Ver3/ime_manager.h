// ime_manager.h - Windows 輸入法衝突管理
#ifndef IME_MANAGER_H
#define IME_MANAGER_H

#include <windows.h>
#include <imm.h>

namespace IMEManager {
    // 初始化輸入法管理器
    void initialize();
    
    // 禁用 Windows 輸入法（切換到英文模式）
    // force: 是否強制執行（忽略已禁用狀態）
    void disableWindowsIME(bool force = false);
    
    // 恢復 Windows 輸入法狀態
    void restoreWindowsIME();
    
    // 檢查當前是否有 Windows 輸入法處於中文模式
    bool isWindowsIMEActive();
    
    // 清理資源
    void cleanup();
}

#endif // IME_MANAGER_H

