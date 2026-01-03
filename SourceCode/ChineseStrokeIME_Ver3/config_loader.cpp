// config_loader.cpp - 設定檔載入實作
#include "config_loader.h"
#include "dictionary.h"
#include "window_manager.h"
#include <fstream>
#include <sstream>
#include <vector>

namespace ConfigLoader {

// 生成默认配置文件
void createDefaultConfigFile() {
    std::ofstream fout("interface_config.ini");
    if (!fout.is_open()) return;
    
    fout << "; interface_config.ini - 配置文件" << std::endl;
    fout << "; 中文筆劃輸入法 介面配置" << std::endl;
    fout << std::endl;
    fout << "[Colors]" << std::endl;
    fout << "; 主視窗顏色" << std::endl;
    fout << "background_color=#F0F0F0" << std::endl;
    fout << "text_color=#000000" << std::endl;
    fout << "selection_color=#0078D7" << std::endl;
    fout << "selection_bg_color=#E6F0FA" << std::endl;
    fout << "error_color=#DC3232" << std::endl;
    fout << std::endl;
    fout << "; 按鈕顏色" << std::endl;
    fout << "close_button_color=#DC3232" << std::endl;
    fout << "close_button_hover_color=#FF4646" << std::endl;
    fout << "mode_button_color=#6432C8" << std::endl;
    fout << "mode_button_hover_color=#7846DC" << std::endl;
    fout << "credits_button_color=#C89632" << std::endl;
    fout << "credits_button_hover_color=#DC9646" << std::endl;
    fout << "refresh_button_color=#32C832" << std::endl;
    fout << "refresh_button_hover_color=#46DC46" << std::endl;
    fout << std::endl;
    fout << "; 候選字視窗顏色" << std::endl;
    fout << "candidate_background_color=#E5FFF8" << std::endl;
    fout << "candidate_text_color=#000000" << std::endl;
    fout << "selected_candidate_bg_color=#F37E7E" << std::endl;
    fout << "selected_candidate_text_color=#01143B" << std::endl;
    fout << std::endl;
    fout << "; 字碼輸入視窗顏色" << std::endl;
    fout << "input_background_color=#FFFFFF" << std::endl;
    fout << "input_text_color=#000000" << std::endl;
    fout << "input_error_text_color=#DC3232" << std::endl;
    fout << "input_hint_text_color=#808080" << std::endl;
    fout << "input_border_color=#808080" << std::endl;
    fout << std::endl;
    fout << "; 暫放視窗顏色" << std::endl;
    fout << "buffer_background_color=#FFFFFF" << std::endl;
    fout << "buffer_text_color=#000000" << std::endl;
    fout << "buffer_cursor_color=#000000" << std::endl;
    fout << std::endl;
    fout << "[Font]" << std::endl;
    fout << "; 主視窗字型" << std::endl;
    fout << "font_size=16" << std::endl;
    fout << "font_name=Microsoft JhengHei" << std::endl;
    fout << std::endl;
    fout << "; 候選字視窗字型" << std::endl;
    fout << "candidate_font_size=23" << std::endl;
    fout << "candidate_font_name=Microsoft JhengHei" << std::endl;
    fout << std::endl;
    fout << "; 字碼輸入視窗字型" << std::endl;
    fout << "input_font_size=20" << std::endl;
    fout << "input_font_name=Microsoft JhengHei" << std::endl;
    fout << std::endl;
    fout << "; 暫放視窗字型" << std::endl;
    fout << "buffer_font_size=20" << std::endl;
    fout << "buffer_font_name=Microsoft JhengHei" << std::endl;
    fout << std::endl;
    fout << "[Window]" << std::endl;
    fout << "; 主視窗大小" << std::endl;
    fout << "window_width=580" << std::endl;
    fout << "window_height=70" << std::endl;
    fout << std::endl;
    fout << "; 候選字視窗大小" << std::endl;
    fout << "candidate_window_width=300" << std::endl;
    fout << "candidate_window_height=320" << std::endl;
    fout << std::endl;
    fout << "; 字碼輸入視窗大小" << std::endl;
    fout << "input_window_width=400" << std::endl;
    fout << "input_window_height=30" << std::endl;
    fout << std::endl;
    fout << "[WindowBehavior]" << std::endl;
    fout << "; 前置維護間隔（毫秒）" << std::endl;
    fout << "topmost_check_interval=5000" << std::endl;
    fout << "; 是否使用強制前置模式" << std::endl;
    fout << "force_stay_on_top=1" << std::endl;
    fout << "; 失去焦點後重新前置的延遲（毫秒）" << std::endl;
    fout << "refocus_delay=50" << std::endl;
    fout << "; 剪貼簿模式開關（0=關閉，1=開啟）" << std::endl;
    fout << "clipboard_mode=0" << std::endl;
    fout << "; 半透明顯示開關（0=關閉，1=開啟）" << std::endl;
    fout << "enable_transparency=0" << std::endl;
    fout << "; 透明度值（0-255，255=完全不透明，0=完全透明）" << std::endl;
    fout << "transparency_alpha=100" << std::endl;
    fout << "; 聯想字功能開關（0=關閉，1=開啟）" << std::endl;
    fout << "enable_word_prediction=0" << std::endl;
    
    fout.close();
}

void loadInterfaceConfig(GlobalState& state) {
    std::ifstream fin("interface_config.ini");
    if (!fin.is_open()) {
        // 配置文件不存在，自动生成默认配置
        createDefaultConfigFile();
        Utils::updateStatus(state, L"已自動生成預設配置文件");
        // 重新尝试打开（如果创建成功）
        fin.open("interface_config.ini");
        if (!fin.is_open()) {
            Utils::updateStatus(state, L"無法創建配置文件，使用預設設定");
            return;
        }
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
                } else if (key == "clipboard_mode") {
                    state.clipboardMode = (value == "1" || value == "true");
                } else if (key == "enable_transparency") {
                    state.enableTransparency = (value == "1" || value == "true");
                } else if (key == "transparency_alpha") {
                    try {
                        int alpha = std::stoi(value);
                        if (alpha >= 0 && alpha <= 255) {
                            state.transparencyAlpha = alpha;
                        }
                    } catch (...) {}
                } else if (key == "enable_word_prediction") {
                    state.enableWordPrediction = (value == "1" || value == "true");
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
    // 載入界面配置
    loadInterfaceConfig(state);
    
    // 載入所有字典和數據文件
    Dictionary::loadMainDict("Zi-Ma-Biao.txt", state);
    Dictionary::loadPunctuator(state);  // 載入標點符號表
    Dictionary::loadPunctMenu(state);    // 載入標點選單
    Dictionary::loadUserDict(state);     // 載入用戶字典
    Dictionary::loadWordPhrases(state);   // 載入詞語庫（用於聯想字功能）
    
    // 更新候選字
    Dictionary::updateCandidates(state);
    
    // 應用透明度設置
    WindowManager::applyTransparency(state);
    
    // 刷新所有視窗
    if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
    if (state.hCandWnd) InvalidateRect(state.hCandWnd, nullptr, TRUE);
    if (state.hInputWnd) InvalidateRect(state.hInputWnd, nullptr, TRUE);
    if (state.hBufferWnd && state.bufferMode) InvalidateRect(state.hBufferWnd, nullptr, TRUE);
    
    Utils::updateStatus(state, L"已重新載入所有配置和字典");
}

void saveInterfaceConfig(const GlobalState& state) {
    // 读取现有配置文件内容，同时提取transparency_alpha的值（如果存在）
    std::ifstream fin("interface_config.ini");
    std::vector<std::string> lines;
    std::string line;
    bool foundClipboardMode = false;
    bool foundEnableTransparency = false;
    bool foundTransparencyAlpha = false;
    bool foundEnableWordPrediction = false;
    std::string currentSection = "";
    int configFileAlpha = state.transparencyAlpha;  // 默认使用state中的值
    
    if (fin.is_open()) {
        while (std::getline(fin, line)) {
            std::string trimmedLine = line;
            trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t\r\n"));
            trimmedLine.erase(trimmedLine.find_last_not_of(" \t\r\n") + 1);
            
            // 检查是否是节标记
            if (trimmedLine.length() > 0 && trimmedLine[0] == '[' && trimmedLine.back() == ']') {
                currentSection = trimmedLine.substr(1, trimmedLine.length() - 2);
                lines.push_back(line);
                continue;
            }
            
            // 检查是否是WindowBehavior节的配置项
            size_t eqPos = trimmedLine.find('=');
            if (eqPos != std::string::npos) {
                std::string key = trimmedLine.substr(0, eqPos);
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                std::string value = trimmedLine.substr(eqPos + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                if (currentSection == "WindowBehavior") {
                    if (key == "clipboard_mode") {
                        // 更新这一行
                        lines.push_back("clipboard_mode=" + (state.clipboardMode ? std::string("1") : std::string("0")));
                        foundClipboardMode = true;
                        continue;
                    } else if (key == "enable_transparency") {
                        lines.push_back("enable_transparency=" + (state.enableTransparency ? std::string("1") : std::string("0")));
                        foundEnableTransparency = true;
                        continue;
                    } else if (key == "transparency_alpha") {
                        // 先尝试读取配置文件中的值
                        if (!value.empty()) {
                            try {
                                int alpha = std::stoi(value);
                                if (alpha >= 0 && alpha <= 255) {
                                    configFileAlpha = alpha;  // 使用配置文件中的值
                                }
                            } catch (...) {}
                        }
                        // 如果配置文件中已有有效值，使用配置文件中的值；否则使用state中的值
                        lines.push_back("transparency_alpha=" + std::to_string(configFileAlpha));
                        foundTransparencyAlpha = true;
                        continue;
                    } else if (key == "enable_word_prediction") {
                        lines.push_back("enable_word_prediction=" + (state.enableWordPrediction ? std::string("1") : std::string("0")));
                        foundEnableWordPrediction = true;
                        continue;
                    }
                }
            }
            
            lines.push_back(line);
        }
        fin.close();
    }
    
    // 如果没找到配置项，需要添加到WindowBehavior节
    bool foundWindowBehavior = false;
    size_t windowBehaviorEnd = 0;
    
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string trimmedLine = lines[i];
        trimmedLine.erase(0, trimmedLine.find_first_not_of(" \t\r\n"));
        trimmedLine.erase(trimmedLine.find_last_not_of(" \t\r\n") + 1);
        
        if (trimmedLine.length() > 0 && trimmedLine[0] == '[' && trimmedLine.back() == ']') {
            std::string section = trimmedLine.substr(1, trimmedLine.length() - 2);
            if (section == "WindowBehavior") {
                foundWindowBehavior = true;
                windowBehaviorEnd = i + 1;
            } else if (foundWindowBehavior) {
                // 在WindowBehavior节的末尾添加缺失的配置项
                std::vector<std::string> toInsert;
                if (!foundClipboardMode) {
                    toInsert.push_back("clipboard_mode=" + (state.clipboardMode ? std::string("1") : std::string("0")));
                }
                if (!foundEnableTransparency) {
                    toInsert.push_back("enable_transparency=" + (state.enableTransparency ? std::string("1") : std::string("0")));
                }
                if (!foundTransparencyAlpha) {
                    toInsert.push_back("transparency_alpha=" + std::to_string(configFileAlpha));
                }
                if (!foundEnableWordPrediction) {
                    toInsert.push_back("enable_word_prediction=" + (state.enableWordPrediction ? std::string("1") : std::string("0")));
                }
                if (!toInsert.empty()) {
                    lines.insert(lines.begin() + i, toInsert.begin(), toInsert.end());
                }
                break;
            }
        } else if (foundWindowBehavior) {
            // 检查是否到了WindowBehavior节的末尾（空行或注释前）
            if (trimmedLine.empty() || trimmedLine[0] == ';' || trimmedLine[0] == '#') {
                // 在最后一个非注释行后插入
                size_t insertPos = i;
                while (insertPos > windowBehaviorEnd && 
                       (lines[insertPos-1].empty() || 
                        (lines[insertPos-1].find_first_not_of(" \t\r\n") != std::string::npos && 
                         (lines[insertPos-1][lines[insertPos-1].find_first_not_of(" \t\r\n")] == ';' || 
                          lines[insertPos-1][lines[insertPos-1].find_first_not_of(" \t\r\n")] == '#')))) {
                    insertPos--;
                }
                std::vector<std::string> toInsert;
                if (!foundClipboardMode) {
                    toInsert.push_back("clipboard_mode=" + (state.clipboardMode ? std::string("1") : std::string("0")));
                }
                if (!foundEnableTransparency) {
                    toInsert.push_back("enable_transparency=" + (state.enableTransparency ? std::string("1") : std::string("0")));
                }
                if (!foundTransparencyAlpha) {
                    toInsert.push_back("transparency_alpha=" + std::to_string(configFileAlpha));
                }
                if (!foundEnableWordPrediction) {
                    toInsert.push_back("enable_word_prediction=" + (state.enableWordPrediction ? std::string("1") : std::string("0")));
                }
                if (!toInsert.empty()) {
                    lines.insert(lines.begin() + insertPos, toInsert.begin(), toInsert.end());
                }
                break;
            }
        }
    }
    
    // 如果WindowBehavior节存在但后面没有其他节，在文件末尾添加
    if (foundWindowBehavior) {
        std::vector<std::string> toInsert;
        if (!foundClipboardMode) {
            toInsert.push_back("clipboard_mode=" + (state.clipboardMode ? std::string("1") : std::string("0")));
        }
        if (!foundEnableTransparency) {
            toInsert.push_back("enable_transparency=" + (state.enableTransparency ? std::string("1") : std::string("0")));
        }
        if (!foundTransparencyAlpha) {
            toInsert.push_back("transparency_alpha=" + std::to_string(state.transparencyAlpha));
        }
        if (!toInsert.empty()) {
            lines.insert(lines.end(), toInsert.begin(), toInsert.end());
        }
    }
    
    // 如果WindowBehavior节不存在，创建一个
    if (!foundWindowBehavior) {
        if (!lines.empty() && !lines.back().empty()) {
            lines.push_back("");
        }
        lines.push_back("[WindowBehavior]");
        lines.push_back("clipboard_mode=" + (state.clipboardMode ? std::string("1") : std::string("0")));
        lines.push_back("enable_transparency=" + (state.enableTransparency ? std::string("1") : std::string("0")));
        lines.push_back("transparency_alpha=" + std::to_string(configFileAlpha));
        lines.push_back("enable_word_prediction=" + (state.enableWordPrediction ? std::string("1") : std::string("0")));
    }
    
    // 写回文件
    std::ofstream fout("interface_config.ini");
    if (fout.is_open()) {
        for (const auto& l : lines) {
            fout << l << std::endl;
        }
        fout.close();
    }
}

void updateTransparencyAlphaFromConfig(GlobalState& state) {
    // 只讀取配置文件中的transparency_alpha值，不修改其他配置
    std::ifstream fin("interface_config.ini");
    if (!fin.is_open()) {
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
        
        if (currentSection == "WindowBehavior") {
            size_t eq = line.find('=');
            if (eq != std::string::npos) {
                std::string key = line.substr(0, eq);
                std::string value = line.substr(eq + 1);
                
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                if (key == "transparency_alpha" && !value.empty()) {
                    try {
                        int alpha = std::stoi(value);
                        if (alpha >= 0 && alpha <= 255) {
                            state.transparencyAlpha = alpha;
                        }
                    } catch (...) {}
                    break;  // 找到後就可以退出
                }
            }
        }
    }
    fin.close();
}

} // namespace ConfigLoader