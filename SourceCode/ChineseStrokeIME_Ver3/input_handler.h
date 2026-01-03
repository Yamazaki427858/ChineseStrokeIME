// input_handler.h - 鍵盤輸入處理
#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include "ime_core.h"

namespace InputHandler {
    // 鍵盤鉤子程序
    LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);
    
    // 輸入模式切換
    void toggleInputMode(GlobalState& state);
    
    // 智慧Enter鍵處理
    void handleEnterKeySmartly(GlobalState& state);
    
    // 筆劃輸入處理
    void processStroke(GlobalState& state, DWORD key);
    
    // 標點符號處理
    void processPunctuator(GlobalState& state, DWORD key);
    
    // 顯示標點選單
    void showPunctMenu(GlobalState& state);
    
    // 文字發送
    void sendTextDirectUnicode(const std::wstring& text);
    
    // 確保目標視窗有焦點（用於文字發送前）
    void ensureTargetWindowFocused();
    
    // 英文字元轉換（全形/半形）
    std::wstring convertEnglishChar(wchar_t ch, bool toFullWidth);
}

#endif // INPUT_HANDLER_H