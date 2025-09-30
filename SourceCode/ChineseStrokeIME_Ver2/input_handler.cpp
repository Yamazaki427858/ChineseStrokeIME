// input_handler.cpp - 輸入處理實作
#include "input_handler.h"
#include "buffer_manager.h"
#include "dictionary.h"
#include "window_manager.h"
#include <windows.h>

extern GlobalState g_state;
extern HHOOK g_hKeyboardHook;

namespace InputHandler {

void sendTextDirectUnicode(const std::wstring& text) {
    if (text.empty()) return;
    
    bool wasBufferFocused = g_state.bufferHasFocus;
    bool wasBufferVisible = g_state.bufferMode && g_state.hBufferWnd && IsWindowVisible(g_state.hBufferWnd);
    RECT originalRect = {0};
    
    if (g_state.bufferMode && g_state.hBufferWnd && wasBufferVisible) {
        GetWindowRect(g_state.hBufferWnd, &originalRect);
        g_state.bufferHasFocus = false;
        ShowWindow(g_state.hBufferWnd, SW_HIDE);
        Sleep(50);
    }
    
    for (wchar_t ch : text) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = 0;
        input.ki.wScan = ch;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        input.ki.time = 0;
        input.ki.dwExtraInfo = 0;
        SendInput(1, &input, sizeof(INPUT));
        
        input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        SendInput(1, &input, sizeof(INPUT));
        
        INPUT flushInput = {0};
        flushInput.type = INPUT_KEYBOARD;
        flushInput.ki.wVk = VK_PACKET;
        flushInput.ki.dwFlags = KEYEVENTF_KEYUP;
        SendInput(1, &flushInput, sizeof(INPUT));
        
        Sleep(8);
    }
    
    if (g_state.bufferMode && g_state.hBufferWnd && wasBufferVisible) {
        Sleep(100);
        
        SetWindowPos(g_state.hBufferWnd, HWND_TOPMOST, 
                    originalRect.left, originalRect.top,
                    originalRect.right - originalRect.left, 
                    originalRect.bottom - originalRect.top,
                    SWP_SHOWWINDOW | SWP_NOACTIVATE);
        
        if (wasBufferFocused) {
            g_state.bufferHasFocus = true;
            SetTimer(g_state.hBufferWnd, 1, 500, NULL);
        }
    }
}

void toggleInputMode(GlobalState& state) {
    state.chineseMode = !state.chineseMode;
    state.input.clear();
    state.candidates.clear();
    state.candidateCodes.clear();
    state.showCand = false;
    state.isInputting = false;
    state.inputError = false;
    state.showPunctMenu = false;
    if (state.hCandWnd) ShowWindow(state.hCandWnd, SW_HIDE);
    if (state.hInputWnd) ShowWindow(state.hInputWnd, SW_HIDE);

    std::wstring modeMsg = state.chineseMode ? L"中文+全形" : L"英文+半形";
    Utils::updateStatus(state, L"Shift切換到" + modeMsg + L"模式");
    if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
}

void handleEnterKeySmartly(GlobalState& state) {
    if (state.bufferMode) {
        if (state.showCand && state.isInputting) {
            // 如果有候選字，選擇第一個
            Dictionary::selectCandidate(state, 0);
            return;
        } else if (!state.showCand && !state.isInputting && !state.bufferText.empty()) {
            // 暫放模式下，無候選字且暫放區有內容時，發送內容
            BufferManager::sendBufferContent(state);
            return;
        }
    } else {
        if (state.showCand && state.isInputting) {
            // 非暫放模式下有候選字時，選擇第一個
            Dictionary::selectCandidate(state, 0);
            return;
        }
    }
}

std::wstring convertEnglishChar(wchar_t ch, bool toFullWidth) {
    if (!toFullWidth) {
        return std::wstring(1, ch);
    }
    
    if (ch >= L'!' && ch <= L'~') {
        return std::wstring(1, ch - L'!' + L'！');
    }
    if (ch == L' ') {
        return L"　";
    }
    
    return std::wstring(1, ch);
}

void processStroke(GlobalState& state, DWORD key) {
    if (!state.chineseMode) return;
    if (key == 'P') {
        showPunctMenu(state);
        return;
    }
    wchar_t inputChar = 0;
    switch (key) {
        case 'U': inputChar = L'u'; break;
        case 'I': inputChar = L'i'; break;
        case 'O': inputChar = L'o'; break;
        case 'J': inputChar = L'j'; break;
        case 'K': inputChar = L'k'; break;
        case 'L': inputChar = L'*'; break;
        case VK_NUMPAD7: inputChar = L'u'; break;
        case VK_NUMPAD8: inputChar = L'i'; break;
        case VK_NUMPAD9: inputChar = L'o'; break;
        case VK_NUMPAD4: inputChar = L'j'; break;
        case VK_NUMPAD5: inputChar = L'k'; break;
        case VK_NUMPAD0: inputChar = L'*'; break;
    }
    if (inputChar) {
        state.input += inputChar;
        Dictionary::updateCandidates(state);
        if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
    }
}

void showPunctMenu(GlobalState& state) {
    state.showPunctMenu = true;
    state.candidates = state.punctCandidates;
    state.candidateCodes.clear();
    for (size_t i = 0; i < state.candidates.size(); i++) {
        state.candidateCodes.push_back(L"P");
    }
    state.selected = 0;
    state.currentPage = 0;
    state.totalPages = (state.candidates.size() + CANDIDATES_PER_PAGE - 1) / CANDIDATES_PER_PAGE;
    state.showCand = true;
    if (state.hCandWnd) {
        ShowWindow(state.hCandWnd, SW_SHOW);
        InvalidateRect(state.hCandWnd, nullptr, TRUE);
    }
    Utils::updateStatus(state, L"全形標點符號選單（按ESC關閉）");
}

void processPunctuator(GlobalState& state, DWORD key) {
    std::wstring punctChar = L"";
    bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    
    switch (key) {
        case VK_OEM_COMMA: punctChar = isShiftPressed ? L"<" : L","; break;
        case VK_OEM_PERIOD: punctChar = isShiftPressed ? L">" : L"."; break;
        case VK_OEM_2: punctChar = isShiftPressed ? L"?" : L"/"; break;
        case '1': if (isShiftPressed) punctChar = L"!"; break;
        case VK_OEM_1: punctChar = isShiftPressed ? L":" : L";"; break;
        case '9': if (isShiftPressed) punctChar = L"("; break;
        case '0': if (isShiftPressed) punctChar = L")"; break;
        case VK_OEM_4: punctChar = isShiftPressed ? L"{" : L"["; break;
        case VK_OEM_6: punctChar = isShiftPressed ? L"}" : L"]"; break;
        case VK_SPACE: punctChar = L" "; break;
        case VK_OEM_7: punctChar = isShiftPressed ? L"\"" : L"'"; break;
        case VK_OEM_MINUS: punctChar = isShiftPressed ? L"_" : L"-"; break;
        case VK_OEM_PLUS: punctChar = isShiftPressed ? L"+" : L"="; break;
        case VK_OEM_5: punctChar = isShiftPressed ? L"|" : L"\\"; break;
        case VK_OEM_3: punctChar = isShiftPressed ? L"~" : L"`"; break;
        case '2': if (isShiftPressed) punctChar = L"@"; break;
        case '3': if (isShiftPressed) punctChar = L"#"; break;
        case '4': if (isShiftPressed) punctChar = L"$"; break;
        case '5': if (isShiftPressed) punctChar = L"%"; break;
        case '6': if (isShiftPressed) punctChar = L"^"; break;
        case '7': if (isShiftPressed) punctChar = L"&"; break;
        case '8': if (isShiftPressed) punctChar = L"*"; break;
    }
    
    if (!punctChar.empty()) {
        // 修正問題1：英文模式下直接發送標點符號，不查表
        if (!state.chineseMode) {
            // 英文模式：直接發送半形標點
            sendTextDirectUnicode(punctChar);
            Utils::updateStatus(state, L"輸入標點：" + punctChar);
            return;
        }
        
        // 中文模式：使用標點表選擇全形或半形
        if (state.punct.count(punctChar)) {
            std::vector<std::wstring> options = state.punct[punctChar];
            if (!options.empty()) {
                std::wstring selectedPunct;
                if (punctChar == L" ") {
                    selectedPunct = L" ";
                } else if (punctChar == L"'") {
                    selectedPunct = state.chineseMode ? L"、" : L"'";
                } else {
                    if (punctChar[0] == L'"' || punctChar == L"[" || punctChar == L"]" || 
                        punctChar == L"{" || punctChar == L"}") {
                        selectedPunct = state.chineseMode ? options[0] : options.back();
                    } else {
                        selectedPunct = state.chineseMode ? options[0] : 
                                       (options.size() > 1 ? options[1] : options[0]);
                    }
                }
                sendTextDirectUnicode(selectedPunct);
                Utils::updateStatus(state, L"輸入標點：" + selectedPunct);
            }
        } else {
            // 如果標點表中沒有，直接發送
            sendTextDirectUnicode(punctChar);
            Utils::updateStatus(state, L"輸入標點：" + punctChar);
        }
    }
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        DWORD key = pKeyboard->vkCode;
        
        // 立即放行更多Ctrl+快捷鍵
         BOOL ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
        if (ctrl && (key == 'C' || key == 'V' || key == 'X' || key == 'Z' || key == 'Y' || 
                    key == 'A' || key == 'S' || key == 'F' || key == 'H' || key == 'N' || 
                    key == 'O' || key == 'P' || key == 'R' || key == 'T' || key == 'W')) {
            return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
        }

        // Shift鍵狀態處理
        if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT) {
            if (wParam == WM_KEYDOWN) {
                if (!g_state.shiftPressed) {
                    g_state.shiftPressed = true;
                    g_state.shiftUsedForCombo = false;
                    g_state.shiftPressTime = GetTickCount();
                }
            } else if (wParam == WM_KEYUP) {
                if (g_state.shiftPressed) {
                    g_state.shiftPressed = false;
                    DWORD pressDuration = GetTickCount() - g_state.shiftPressTime;
                    if (!g_state.shiftUsedForCombo && pressDuration < 500 && pressDuration > 30) {
                        toggleInputMode(g_state);
                    }
                    g_state.shiftUsedForCombo = false;
                }
            }
            return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
        }
        
        if (wParam == WM_KEYDOWN) {
            if (g_state.shiftPressed && key != VK_SHIFT && key != VK_LSHIFT && key != VK_RSHIFT) {
                g_state.shiftUsedForCombo = true;
            }

            if (key == VK_ESCAPE) {
    // 只有當輸入法真的需要處理ESC時才攔截
    if (g_state.showCand || g_state.showPunctMenu || g_state.isInputting) {
        // 有候選字、標點選單或正在輸入時，由輸入法處理
        PostMessage(g_state.hWnd, WM_USER+100, VK_ESCAPE, 0);
        return 1;
    }
    
    // 暫放模式下有焦點時的特殊處理
    if (g_state.bufferMode && g_state.bufferHasFocus) {
        // 讓ESC鍵通過到目標應用程式
        return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
    }
    
    // 其他情況：輸入法沒有在使用，讓ESC鍵通過給其他程式
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

            // 數字鍵優先用於選字
            if (key >= '1' && key <= '9') {
                if (g_state.showCand && g_state.isInputting) {
                    PostMessage(g_state.hWnd, WM_USER+100, key, 0);
                    return 1;
                }
            }

            // 暫放視窗焦點判斷
            bool isBufferWindowActive = false;
            if (g_state.bufferMode && g_state.hBufferWnd) {
                HWND foregroundWnd = GetForegroundWindow();
                isBufferWindowActive = (IsWindowVisible(g_state.hBufferWnd) && 
                                       g_state.bufferHasFocus &&
                                       (foregroundWnd == g_state.hWnd || foregroundWnd == g_state.hBufferWnd));
            }

            // 暫放模式下攔截邏輯
            if (g_state.bufferMode) {
                bool shouldInterceptForBuffer = false;
                
                if (isBufferWindowActive) {
                    shouldInterceptForBuffer = true;
                } else {
                    shouldInterceptForBuffer = (
                        (key >= 'A' && key <= 'Z') ||
                        (key >= '0' && key <= '9' && !g_state.showCand) ||
                        (key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || 
                         key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0) ||
                        (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L') ||
                        (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 ||
                         key == VK_OEM_1 || key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 ||
                         key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||
                         key == VK_SPACE)
                    );
                }
                
                if (shouldInterceptForBuffer) {
                    if (!isBufferWindowActive && g_state.hBufferWnd) {
                        g_state.bufferHasFocus = true;
                        SetTimer(g_state.hBufferWnd, 1, 500, NULL);
                        InvalidateRect(g_state.hBufferWnd, nullptr, TRUE);
                        isBufferWindowActive = true;
                    }
                }
            }

            // 暫放視窗輸入處理
            if (isBufferWindowActive) {
                // 方向鍵控制
                if (key == VK_LEFT || key == VK_RIGHT || key == VK_HOME || key == VK_END) {
                    switch(key) {
                        case VK_LEFT: BufferManager::moveCursor(g_state, -1); break;
                        case VK_RIGHT: BufferManager::moveCursor(g_state, 1); break;
                        case VK_HOME: 
                            g_state.bufferCursorPos = 0;
                            if (g_state.hBufferWnd) InvalidateRect(g_state.hBufferWnd, nullptr, TRUE);
                            break;
                        case VK_END: 
                            g_state.bufferCursorPos = g_state.bufferText.length();
                            if (g_state.hBufferWnd) InvalidateRect(g_state.hBufferWnd, nullptr, TRUE);
                            break;
                    }
                    return 1;
                }
                
                // Backspace處理
                if (key == VK_BACK) {
                    if (g_state.shiftPressed) {
                        BufferManager::deleteCharAtCursor(g_state, false);
                        return 1;
                    } else if (g_state.isInputting && !g_state.input.empty()) {
                        PostMessage(g_state.hWnd, WM_USER+100, VK_BACK, 0);
                        return 1;
                    } else if (!g_state.bufferText.empty()) {
                        BufferManager::deleteCharAtCursor(g_state, false);
                        return 1;
                    }
                    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
                }
                
                // Delete處理
                if (key == VK_DELETE) {
                    if (!g_state.bufferText.empty()) {
                        BufferManager::deleteCharAtCursor(g_state, true);
                        return 1;
                    }
                    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
                }
                
                // NumPad筆劃輸入支援
                bool isNumpadStrokeKey = (key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || 
                                         key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0);
                if (isNumpadStrokeKey && g_state.chineseMode) {
                    PostMessage(g_state.hWnd, WM_USER+100, key, 0);
                    return 1;
                }
				
				// 新增：處理所有 Numpad 數字鍵的直接輸入
				if ((key >= VK_NUMPAD0 && key <= VK_DIVIDE)) {
    // 中文模式下，某些 Numpad 鍵用於筆劃輸入
    if (g_state.chineseMode && !g_state.showCand) {
        bool isStrokeKey = (key == VK_NUMPAD7 || key == VK_NUMPAD8 || 
                           key == VK_NUMPAD9 || key == VK_NUMPAD4 || 
                           key == VK_NUMPAD5 || key == VK_NUMPAD0);
        if (isStrokeKey) {
            PostMessage(g_state.hWnd, WM_USER+100, key, 0);
            return 1;
        }
    }
    
    // 其他情況下直接輸入對應字符
    wchar_t ch = 0;
    switch(key) {
        // 數字鍵
        case VK_NUMPAD0: ch = L'0'; break;
        case VK_NUMPAD1: ch = L'1'; break;
        case VK_NUMPAD2: ch = L'2'; break;
        case VK_NUMPAD3: ch = L'3'; break;
        case VK_NUMPAD4: ch = L'4'; break;
        case VK_NUMPAD5: ch = L'5'; break;
        case VK_NUMPAD6: ch = L'6'; break;
        case VK_NUMPAD7: ch = L'7'; break;
        case VK_NUMPAD8: ch = L'8'; break;
        case VK_NUMPAD9: ch = L'9'; break;
        // 運算符和標點
        case VK_MULTIPLY: ch = L'*'; break;
        case VK_ADD: ch = L'+'; break;
        case VK_SUBTRACT: ch = L'-'; break;
        case VK_DECIMAL: ch = L'.'; break;
        case VK_DIVIDE: ch = L'/'; break;
    }
    
    if (ch != 0) {
        BufferManager::insertTextAtCursor(g_state, std::wstring(1, ch));
        return 1;
    }
}
                
                // 普通筆劃輸入（包含P鍵用於標點選單）
                bool isStrokeKey = (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L' || key == 'P');
                if (isStrokeKey && g_state.chineseMode) {
                    PostMessage(g_state.hWnd, WM_USER+100, key, 0);
                    return 1;
                }
                
                // Shift+數字標點處理
                if (g_state.shiftPressed && ((key >= '0' && key <= '9') || key == VK_OEM_COMMA || key == VK_OEM_PERIOD)) {
                    std::wstring punctChar = L"";
                    switch(key) {
                        case '1': punctChar = g_state.chineseMode ? L"！" : L"!"; break;
                        case '2': punctChar = g_state.chineseMode ? L"＠" : L"@"; break;
                        case '3': punctChar = g_state.chineseMode ? L"＃" : L"#"; break;
                        case '4': punctChar = g_state.chineseMode ? L"＄" : L"$"; break;
                        case '5': punctChar = g_state.chineseMode ? L"％" : L"%"; break;
                        case '6': punctChar = g_state.chineseMode ? L"⌃" : L"^"; break;
                        case '7': punctChar = g_state.chineseMode ? L"＆" : L"&"; break;
                        case '8': punctChar = g_state.chineseMode ? L"＊" : L"*"; break;
                        case '9': punctChar = g_state.chineseMode ? L"（" : L"("; break;
                        case '0': punctChar = g_state.chineseMode ? L"）" : L")"; break;
                        case VK_OEM_COMMA: punctChar = g_state.chineseMode ? L"《" : L"<"; break;
                        case VK_OEM_PERIOD: punctChar = g_state.chineseMode ? L"》" : L">"; break;
                    }
                    if (!punctChar.empty()) {
                        BufferManager::insertTextAtCursor(g_state, punctChar);
                        return 1;
                    }
                }
                
                // 數字輸入（沒有候選字時）
                if ((key >= '0' && key <= '9' && !g_state.shiftPressed && !g_state.showCand)) {
					// 暫放模式下數字一律輸入半形
					wchar_t halfWidthNum = key;
					std::wstring converted(1, halfWidthNum);
					BufferManager::insertTextAtCursor(g_state, converted);
                    return 1;
                }
                
                // 英文字母輸入（排除中文模式下的P鍵）
                if (key >= 'A' && key <= 'Z') {
                    // 中文模式下P鍵用於標點符號選單，不當作普通字母處理
                    if (key == 'P' && g_state.chineseMode) {
                        PostMessage(g_state.hWnd, WM_USER+100, key, 0);
                        return 1;
                    }
                    
                    bool isCapital = ((GetKeyState(VK_CAPITAL) & 0x0001) != 0) ^ g_state.shiftPressed;
                    wchar_t ch = key;
                    if (!isCapital) {
                        ch = key + 32;
                    }
						// 暫放模式下英文字母一律輸入半形
						std::wstring converted(1, ch);
						BufferManager::insertTextAtCursor(g_state, converted);
						return 1;
                }
                
                // 標點符號處理
                bool isPunctKey = (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 ||
                                  key == VK_OEM_1 || key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 ||
                                  key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||
                                  key == VK_SPACE);
                
                if (isPunctKey && !g_state.shiftPressed) {
                    std::wstring punctChar = L"";
                    
                    switch (key) {
                        case VK_SPACE: punctChar = L" "; break; // 暫放模式下空白鍵一律輸入半形空格
                        case VK_OEM_COMMA: punctChar = g_state.chineseMode ? L"，" : L","; break;
                        case VK_OEM_PERIOD: punctChar = g_state.chineseMode ? L"。" : L"."; break;
                        case VK_OEM_2: punctChar = g_state.chineseMode ? L"／" : L"/"; break;
                        case VK_OEM_1: punctChar = g_state.chineseMode ? L"；" : L";"; break;
                        case VK_OEM_4: punctChar = g_state.chineseMode ? L"「" : L"["; break;
                        case VK_OEM_6: punctChar = g_state.chineseMode ? L"」" : L"]"; break;
                        case VK_OEM_7: punctChar = g_state.chineseMode ? L"、" : L"'"; break;
                        case VK_OEM_MINUS: punctChar = g_state.chineseMode ? L"－" : L"-"; break;
                        case VK_OEM_PLUS: punctChar = g_state.chineseMode ? L"＝" : L"="; break;
                        case VK_OEM_5: punctChar = g_state.chineseMode ? L"＼" : L"\\"; break;
                        case VK_OEM_3: punctChar = g_state.chineseMode ? L"`" : L"`"; break;
                    }
					
					// 特別處理：暫放模式下空白鍵強制輸入半形空格
					if (key == VK_SPACE) {
					punctChar = L" ";
					}
                    
                    if (!punctChar.empty()) {
                        BufferManager::insertTextAtCursor(g_state, punctChar);
                        return 1;
                    }
                }
                
                // Shift+其他標點符號
                if (g_state.shiftPressed && isPunctKey) {
                    std::wstring punctChar = L"";
                    
                        switch (key) {
							case VK_OEM_2: punctChar = g_state.chineseMode ? L"？" : L"?"; break;
							case VK_OEM_1: punctChar = g_state.chineseMode ? L"：" : L":"; break;
							case VK_OEM_4: punctChar = g_state.chineseMode ? L"『" : L"{"; break;
							case VK_OEM_6: punctChar = g_state.chineseMode ? L"』" : L"}"; break;
							case VK_OEM_7: punctChar = g_state.chineseMode ? L"＂" : L"\""; break;
							case VK_OEM_MINUS: punctChar = g_state.chineseMode ? L"＿" : L"_"; break;
							case VK_OEM_PLUS: punctChar = g_state.chineseMode ? L"＋" : L"+"; break;
							case VK_OEM_5: punctChar = g_state.chineseMode ? L"｜" : L"|"; break;
							case VK_OEM_3: punctChar = g_state.chineseMode ? L"～" : L"~"; break;
						}
    
						if (!punctChar.empty()) {
							BufferManager::insertTextAtCursor(g_state, punctChar);
							return 1;
                    }
                }
            }

            // Enter鍵處理
            if (key == VK_RETURN) {
                if (isBufferWindowActive && !g_state.showCand && !g_state.isInputting && !g_state.bufferText.empty()) {
                    PostMessage(g_state.hWnd, WM_USER+100, VK_RETURN, 0);
                    return 1;
                }
            }

            // 正常中文輸入處理
            if (!g_state.bufferMode || !isBufferWindowActive) {
                bool isStrokeKey = (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L' || key == 'P' ||
                                    key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || 
                                    key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0);

                bool isPunctKey = (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 ||
                       key == VK_OEM_1 || key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 ||
                       key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||
                       (key == '1' && g_state.shiftPressed) || (key == '2' && g_state.shiftPressed) || 
                       (key == '3' && g_state.shiftPressed) || (key == '4' && g_state.shiftPressed) || 
                       (key == '5' && g_state.shiftPressed) || (key == '6' && g_state.shiftPressed) || 
                       (key == '7' && g_state.shiftPressed) || (key == '8' && g_state.shiftPressed) || 
                       (key == '9' && g_state.shiftPressed) || (key == '0' && g_state.shiftPressed));
                
                bool isFunctionKey = ((g_state.isInputting || g_state.showPunctMenu) && 
                          (key == VK_SPACE || key == VK_BACK || key == VK_ESCAPE ||
                           key == VK_UP || key == VK_DOWN || key == VK_TAB ||
                           (key >= '1' && key <= '9')));

                if (g_state.chineseMode) {
                    // 中文模式：攔截筆劃鍵、標點符號和功能鍵
                    if (isStrokeKey || isPunctKey || (key == VK_SPACE && g_state.isInputting) || isFunctionKey) {
                        PostMessage(g_state.hWnd, WM_USER+100, key, 0);
                        return 1;
                    }
                } else {
                    // 英文模式：只在有候選字或輸入狀態時才攔截功能鍵
                    // 標點符號直接放行，讓系統處理
                    if (isFunctionKey) {
                         PostMessage(g_state.hWnd, WM_USER+100, key, 0);
                         return 1;
                    }
                    // 英文模式下標點符號直接放行，不攔截
                }
            }
        }
        else if (wParam == WM_KEYUP) {
            if (key != VK_SHIFT && key != VK_LSHIFT && key != VK_RSHIFT && !g_state.shiftPressed) {
                g_state.shiftUsedForCombo = false;
            }
        }
    }
    
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

} // namespace InputHandler
