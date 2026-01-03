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
	 // 新增：文字選取功能
    void startSelection(GlobalState& state, int x, int y);
    void updateSelection(GlobalState& state, int x, int y);
    void endSelection(GlobalState& state);
    void clearSelection(GlobalState& state);
    
    // 選取操作
    void selectAll(GlobalState& state);
    void copySelection(GlobalState& state);
    void cutSelection(GlobalState& state);
    void deleteSelection(GlobalState& state);
    
    // 剪貼簿模式：從剪貼簿貼上文字（貼上後清空剪貼簿）
    void pasteFromClipboard(GlobalState& state);
    
    // 剪貼簿模式：更新剪貼簿（實時複製暫放文字）
    void updateClipboardInMode(GlobalState& state);
    
    // 剪貼簿模式：檢查輸入結束並更新狀態
    void checkInputEnd(GlobalState& state);
    
    // 處理Ctrl+V貼上後清空暫放視窗和剪貼簿
    void handleClipboardPaste(GlobalState& state);
    
    // 座標轉換
    int getTextPositionFromPoint(const GlobalState& state, int x, int y);
    POINT getPointFromTextPosition(const GlobalState& state, int position);
    
    // 獲取選取的文字
    std::wstring getSelectedText(const GlobalState& state);
	
	// 歷史記錄管理
    void saveSnapshot(GlobalState& state);
    void undo(GlobalState& state);
    void redo(GlobalState& state);
    void clearHistory(GlobalState& state);
	
}

   


#endif // BUFFER_MANAGER_H
