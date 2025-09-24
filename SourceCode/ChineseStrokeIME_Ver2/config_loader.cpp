// config_loader.cpp - 設定檔載入實作
#include "config_loader.h"
#include "dictionary.h"
#include <fstream>
#include <sstream>

namespace ConfigLoader {

void loadInterfaceConfig(GlobalState& state) {
    std::ifstream fin("interface_config.ini");
    if (!fin.is_open()) {
        Utils::updateStatus(state, L"使用預設介面配色");
        return;
    }
    
    std::string line;
    std::string currentSection = "";
    
    while (std::getline(fin, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        size_t eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = line.substr(0, eq);
            std::string value = line.substr(eq + 1);
            
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            if (currentSection == "Colors") {
                if (key == "background_color") {
                    state.bgColor = Utils::parseColorFromString(value);
                } else if (key == "text_color") {
                    state.textColor = Utils::parseColorFromString(value);
                } else if (key == "selection_color") {
                    state.selColor = Utils::parseColorFromString(value);
                } else if (key == "selection_bg_color") {
                    state.selBgColor = Utils::parseColorFromString(value);
                } else if (key == "error_color") {
                    state.errorColor = Utils::parseColorFromString(value);
                } else if (key == "close_button_color") {
                    state.closeButtonColor = Utils::parseColorFromString(value);
                } else if (key == "close_button_hover_color") {
                    state.closeButtonHoverColor = Utils::parseColorFromString(value);
                } else if (key == "mode_button_color") {
                    state.modeButtonColor = Utils::parseColorFromString(value);
                } else if (key == "mode_button_hover_color") {
                    state.modeButtonHoverColor = Utils::parseColorFromString(value);
                } else if (key == "credits_button_color") {
                    state.creditsButtonColor = Utils::parseColorFromString(value);
                } else if (key == "credits_button_hover_color") {
                    state.creditsButtonHoverColor = Utils::parseColorFromString(value);
                } else if (key == "refresh_button_color") {
                    state.refreshButtonColor = Utils::parseColorFromString(value);
                } else if (key == "refresh_button_hover_color") {
                    state.refreshButtonHoverColor = Utils::parseColorFromString(value);
                } else if (key == "candidate_background_color") {
                    state.candidateBackgroundColor = Utils::parseColorFromString(value);
                } else if (key == "candidate_text_color") {
                    state.candidateTextColor = Utils::parseColorFromString(value);
                } else if (key == "selected_candidate_bg_color") {
                    state.selectedCandidateBackgroundColor = Utils::parseColorFromString(value);
                } else if (key == "selected_candidate_text_color") {
                    state.selectedCandidateTextColor = Utils::parseColorFromString(value);
                // 字碼輸入視窗顏色（新增）
                } else if (key == "input_background_color") {
                    state.inputBackgroundColor = Utils::parseColorFromString(value);
                } else if (key == "input_text_color") {
                    state.inputTextColor = Utils::parseColorFromString(value);
                } else if (key == "input_error_text_color") {
                    state.inputErrorTextColor = Utils::parseColorFromString(value);
                } else if (key == "input_hint_text_color") {
                    state.inputHintTextColor = Utils::parseColorFromString(value);
                } else if (key == "input_border_color") {
                    state.inputBorderColor = Utils::parseColorFromString(value);
                // 暫放視窗顏色
                } else if (key == "buffer_background_color") {
                    state.bufferBackgroundColor = Utils::parseColorFromString(value);
                } else if (key == "buffer_text_color") {
                    state.bufferTextColor = Utils::parseColorFromString(value);
                } else if (key == "buffer_cursor_color") {
                    state.bufferCursorColor = Utils::parseColorFromString(value);
                }
            } else if (currentSection == "Font") {
                if (key == "font_size") {
                    try {
                        int fontSize = std::stoi(value);
                        if (fontSize >= 8 && fontSize <= 72) {
                            state.fontSize = fontSize;
                        }
                    } catch (...) {}
                } else if (key == "font_name") {
                    state.fontName = Utils::utf8ToWstr(value);
                } else if (key == "candidate_font_size") {
                    try {
                        int candidateFontSize = std::stoi(value);
                        if (candidateFontSize >= 8 && candidateFontSize <= 72) {
                            state.candidateFontSize = candidateFontSize;
                        }
                    } catch (...) {}
                } else if (key == "candidate_font_name") {
                    state.candidateFontName = Utils::utf8ToWstr(value);
                // 字碼輸入視窗字型（新增）
                } else if (key == "input_font_size") {
                    try {
                        int inputFontSize = std::stoi(value);
                        if (inputFontSize >= 8 && inputFontSize <= 72) {
                            state.inputFontSize = inputFontSize;
                        }
                    } catch (...) {}
                } else if (key == "input_font_name") {
                    state.inputFontName = Utils::utf8ToWstr(value);
                // 暫放視窗字型
                } else if (key == "buffer_font_size") {
                    try {
                        int bufferFontSize = std::stoi(value);
                        if (bufferFontSize >= 8 && bufferFontSize <= 72) {
                            state.bufferFontSize = bufferFontSize;
                        }
                    } catch (...) {}
                } else if (key == "buffer_font_name") {
                    state.bufferFontName = Utils::utf8ToWstr(value);
                }
            } else if (currentSection == "Window") {
                if (key == "window_width") {
                    try {
                        int windowWidth = std::stoi(value);
                        if (windowWidth >= 300 && windowWidth <= 1000) {
                            state.windowWidth = windowWidth;
                        }
                    } catch (...) {}
                } else if (key == "window_height") {
                    try {
                        int windowHeight = std::stoi(value);
                        if (windowHeight >= 50 && windowHeight <= 200) {
                            state.windowHeight = windowHeight;
                        }
                    } catch (...) {}
                } else if (key == "candidate_window_width") {
                    try {
                        int candidateWidth = std::stoi(value);
                        if (candidateWidth >= 200 && candidateWidth <= 1000) {
                            state.candidateWidth = candidateWidth;
                        }
                    } catch (...) {}
                } else if (key == "candidate_window_height") {
                    try {
                        int candidateHeight = std::stoi(value);
                        if (candidateHeight >= 100 && candidateHeight <= 600) {
                            state.candidateHeight = candidateHeight;
                        }
                    } catch (...) {}
                // 字碼輸入視窗尺寸（新增）
                } else if (key == "input_window_width") {
                    try {
                        int inputWidth = std::stoi(value);
                        if (inputWidth >= 200 && inputWidth <= 800) {
                            state.inputWindowWidth = inputWidth;
                        }
                    } catch (...) {}
                } else if (key == "input_window_height") {
                    try {
                        int inputHeight = std::stoi(value);
                        if (inputHeight >= 20 && inputHeight <= 100) {
                            state.inputWindowHeight = inputHeight;
                        }
                    } catch (...) {}
                }
            } else if (currentSection == "WindowBehavior") {
                if (key == "topmost_check_interval") {
                    try {
                        int interval = std::stoi(value);
                        if (interval >= 1000 && interval <= 60000) {  // 1秒到60秒
                            state.topmostCheckInterval = interval;
                        }
                    } catch (...) {}
                } else if (key == "force_stay_on_top") {
                    state.forceStayOnTop = (value == "1" || value == "true");
                } else if (key == "refocus_delay") {
                    try {
                        int delay = std::stoi(value);
                        if (delay >= 0 && delay <= 1000) {  // 0到1秒
                            state.refocusDelay = delay;
                        }
                    } catch (...) {}
                }
            } else if (currentSection == "InputSettings") {
                if (key == "auto_wildcard_length") {
                    try {
                        int length = std::stoi(value);
                        if (length >= 6 && length <= 20) {
                            // 可以添加到 GlobalState 中使用
                        }
                    } catch (...) {}
                } else if (key == "suggest_3plus3_length") {
                    try {
                        int length = std::stoi(value);
                        if (length >= 6 && length <= 15) {
                            // 可以添加到 GlobalState 中使用
                        }
                    } catch (...) {}
                }
            } else if (currentSection == "MultiScreenSettings") {
                if (key == "show_screen_change_notification") {
                    // 處理螢幕變更通知設定
                    // state.showScreenChangeNotification = (value == "1" || value == "true");
                }
            }
        }
    }
    fin.close();
    Utils::updateStatus(state, L"重新載入介面配置（含字碼視窗配色）");
}

void loadAllConfigs(GlobalState& state) {
    loadInterfaceConfig(state);
    Dictionary::loadPunctMenu(state);
    Utils::updateStatus(state, L"載入設定檔完成");
}

void refreshConfigs(GlobalState& state) {
    loadInterfaceConfig(state);
    Dictionary::loadMainDict("Zi-Ma-Biao.txt", state);
    Dictionary::loadPunctMenu(state);
    Dictionary::loadUserDict(state);
    Dictionary::updateCandidates(state);
    if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
    if (state.hCandWnd) InvalidateRect(state.hCandWnd, nullptr, TRUE);
    if (state.hInputWnd) InvalidateRect(state.hInputWnd, nullptr, TRUE); // 新增：刷新字碼視窗
    if (state.hBufferWnd && state.bufferMode) InvalidateRect(state.hBufferWnd, nullptr, TRUE);
    Utils::updateStatus(state, L"已重新載入所有配置和字典");
}

} // namespace ConfigLoader