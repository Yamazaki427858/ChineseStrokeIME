// buffer_manager.h - 暫放視窗管理
#ifndef BUFFER_MANAGER_H
#define BUFFER_MANAGER_H

#include "ime_core.h"

namespace BufferManager {
    // 暫放模式控制
    void toggleBufferMode(GlobalState& state);
    
    // 暫放視窗操作
    int calculateBufferWindowHeight(const GlobalState& state);
    void saveBufferToFile(const GlobalState& state);
    void loadBufferFromFile(GlobalState& state);
    void saveBufferToTimestampedFile(const GlobalState& state);
    
    // 暫放內容操作
    void sendBufferContent(GlobalState& state);
    void clearBufferWithConfirm(GlobalState& state);
    
    // 文字編輯操作
    void insertTextAtCursor(GlobalState& state, const std::wstring& text);
    void deleteCharAtCursor(GlobalState& state, bool forward = false);
    void moveCursor(GlobalState& state, int direction);
    void setCursorPosition(GlobalState& state, int x, int y);
}

#endif // BUFFER_MANAGER_H
