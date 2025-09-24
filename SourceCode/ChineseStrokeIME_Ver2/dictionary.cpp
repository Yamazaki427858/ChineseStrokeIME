// dictionary.cpp - 字典管理實作（修正字碼表持續顯示和3+3提示）
#include "dictionary.h"
#include "buffer_manager.h"
#include "input_handler.h"
#include "window_manager.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctime>

namespace Dictionary {

// 新增：增強型輸入驗證（參考OptimizedChineseStrokeIME.cpp）
bool enhancedValidateInput(const std::wstring& input) {
    if (input.empty()) return true;
    if (input.length() > 30) return false;  // 防止過長輸入
    
    int validCharCount = 0;
    for (wchar_t ch : input) {
        if (ch == L'u' || ch == L'i' || ch == L'o' || ch == L'j' || ch == L'k' || ch == L'*') {
            validCharCount++;
        }
    }
    
    return validCharCount > 0;
}

// 新增：過濾有效字符（參考OptimizedChineseStrokeIME.cpp）
std::wstring filterValidChars(const std::wstring& input) {
    std::wstring filtered;
    for (wchar_t ch : input) {
        if (ch == L'u' || ch == L'i' || ch == L'o' || ch == L'j' || ch == L'k' || ch == L'*') {
            filtered += ch;
        }
    }
    return filtered;
}

// 新增：獲取輸入顯示內容（包含3+3提示）
std::wstring getInputDisplay(const GlobalState& state) {
    std::wstring display = state.input;
    
    if (state.showPunctMenu) {
        display = L"標點符號選單";
    } else if (!state.input.empty()) {
        std::wstring filtered = filterValidChars(state.input);
        
        // 如果原輸入包含無效字符，顯示過濾結果
        if (filtered != state.input) {
            display += L" [已過濾: " + filtered + L"]";
        }
        
        // 3+3模式提示（第七個字碼開始提示）
        if (filtered.length() >= 7) {
            std::wstring first3 = filtered.substr(0, 3);
            std::wstring last3 = filtered.substr(filtered.length() - 3);
            display += L" (建議: " + first3 + L"*" + last3 + L")";
        } else if (filtered.length() > 6) {
            display += L" (可用*號導出)";
        } else if (filtered.length() > 3) {
            display += L" (可用*號搜尋)";
        }
    }
    
    return display;
}

double calculateTimeWeight(time_t lastUsed) {
    time_t now = time(nullptr);
    double daysDiff = difftime(now, lastUsed) / (24 * 3600);
    if (daysDiff <= 1) return 1.0;
    if (daysDiff <= 7) return 0.8;
    if (daysDiff <= 30) return 0.6;
    if (daysDiff <= 90) return 0.4;
    return 0.2;
}

double getWordScore(const GlobalState& state, const std::wstring& word, const std::wstring& code) {
    double score = (10.0 - code.length()) * 2.0;
    if (state.wordFreq.find(word) != state.wordFreq.end()) {
        const WordInfo& info = state.wordFreq.at(word);
        double freqScore = info.frequency * 1.0;
        double timeWeight = calculateTimeWeight(info.lastUsed);
        double permanentBonus = info.isPermanent ? 5.0 : 0.0;
        score += (freqScore * timeWeight) + permanentBonus;
    }
    if (!state.lastSelected.empty() && state.contextLearning.find(state.lastSelected) != state.contextLearning.end()) {
        const auto& context = state.contextLearning.at(state.lastSelected);
        if (std::find(context.begin(), context.end(), word) != context.end()) {
            score += 3.0;
        }
    }
    return score;
}

void learnWord(GlobalState& state, const std::wstring& word) {
    if (Utils::isPunctuation(word)) return;
    if (word.empty()) return;
    
    time_t now = time(nullptr);
    if (state.wordFreq.find(word) == state.wordFreq.end()) {
        state.wordFreq[word] = {1, now, 1, false};
        Utils::updateStatus(state, L"學習新詞：" + word + L"（暫存）");
    } else {
        WordInfo& info = state.wordFreq[word];
        info.frequency++;
        info.lastUsed = now;
        if (!info.isPermanent) {
            info.tempCount++;
            if (info.tempCount >= 3) {
                info.isPermanent = true;
                Utils::updateStatus(state, L"詞語加入永久詞庫：" + word);
            } else {
                Utils::updateStatus(state, L"詞語學習中：" + word + L"（" + std::to_wstring(info.tempCount) + L"/3）");
            }
        }
    }
    
    if (!state.lastSelected.empty() && state.lastSelected != word) {
        state.contextLearning[state.lastSelected].push_back(word);
        if (state.contextLearning[state.lastSelected].size() > 10) {
            state.contextLearning[state.lastSelected].erase(state.contextLearning[state.lastSelected].begin());
        }
    }
    state.lastSelected = word;
}

void loadMainDict(const char* filename, GlobalState& state) {
    state.dict.clear();
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        Utils::updateStatus(state, L"無法載入字典檔案，使用內建字典");
        state.dict[L"u"] = {L"一"};
        state.dict[L"i"] = {L"丨"};
        state.dict[L"o"] = {L"丿"};
        state.dict[L"j"] = {L"丶"};
        state.dict[L"k"] = {L"乙"};
        state.dictSize = 5;
        return;
    }
    
    std::string line;
    int count = 0;
    while (std::getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t tab = line.find('\t');
        if (tab == std::string::npos) continue;
        std::wstring key = Utils::utf8ToWstr(line.substr(tab+1));
        std::wstring val = Utils::utf8ToWstr(line.substr(0, tab));
        if (!key.empty() && !val.empty()) {
            state.dict[key].push_back(val);
            count++;
        }
    }
    fin.close();
    state.dictSize = count;
    Utils::updateStatus(state, L"重新載入中文字典：" + std::to_wstring(count) + L" 個字");
}

void loadPunctuator(GlobalState& state) {
    state.punct[L","] = {L"，", L","};
    state.punct[L"."] = {L"。", L"."};
    state.punct[L"?"] = {L"？", L"?"};
    state.punct[L"!"] = {L"！", L"!"};
    state.punct[L":"] = {L"：", L":"};
    state.punct[L";"] = {L"；", L";"};
    state.punct[L"("] = {L"（", L"("};
    state.punct[L")"] = {L"）", L")"};
    state.punct[L"["] = {L"「", L"「", L"［", L"["};
    state.punct[L"]"] = {L"」", L"」", L"］", L"]"};
    state.punct[L"{"] = {L"『", L"{"};
    state.punct[L"}"] = {L"』", L"}"};
    state.punct[L" "] = {L" "};
    state.punct[L"<"] = {L"《", L"<"};
    state.punct[L">"] = {L"》", L">"};
    state.punct[L"/"] = {L"／", L"/"};
    state.punct[L"'"] = {L"、", L"'"};
    state.punct[L"-"] = {L"－", L"-"};
    state.punct[L"_"] = {L"＿", L"_"};
    state.punct[L"="] = {L"＝", L"="};
    state.punct[L"\\"] = {L"＼", L"\\"};
    state.punct[L"|"] = {L"｜", L"|"}; 
    state.punct[L"~"] = {L"～", L"~"}; 
    state.punct[L"`"] = {L"`", L"`"};
    state.punct[L"^"] = {L"⌃", L"^"};
    state.punct[L"&"] = {L"＆", L"&"}; 
    state.punct[L"*"] = {L"＊", L"*"}; 
    state.punct[L"+"] = {L"＋", L"+"};
    state.punct[L"#"] = {L"＃", L"#"};
    state.punct[L"@"] = {L"＠", L"@"};   
    state.punct[L"$"] = {L"＄", L"$"}; 
    state.punct[L"%"] = {L"％", L"%"};
    state.punct[L"\""] = {L"＂", L"\""};
	
	
}

void loadPunctMenu(GlobalState& state) {
    state.punctCandidates.clear();
    
    // 首先嘗試載入檔案
    std::ifstream fin("punct_menu.txt", std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        Utils::updateStatus(state, L"無法開啟 punct_menu.txt，使用內建標點選單");
    } else {
        std::string content((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        fin.close();
        
        // 處理 UTF-8 BOM
        if (content.length() >= 3 && 
            content[0] == static_cast<char>(0xEF) &&
            content[1] == static_cast<char>(0xBB) &&
            content[2] == static_cast<char>(0xBF)) {
            content = content.substr(3);
        }
        
        // 按行分割處理
        std::stringstream ss(content);
        std::string line;
        int count = 0;
        
        while (std::getline(ss, line)) {
            // 移除行尾的 \r（Windows 換行符）
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            // 移除前後空格
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            // 跳過空行和註解行
            if (line.empty() || line[0] == '#') continue;
            
            // 轉換為寬字符
            try {
                std::wstring punct = Utils::utf8ToWstr(line);
                if (!punct.empty()) {
                    state.punctCandidates.push_back(punct);
                    count++;
                }
            } catch (...) {
                // 轉換失敗，跳過這行
                continue;
            }
        }
        
        if (count >= 5) {
            Utils::updateStatus(state, L"載入標點符號選單：" + std::to_wstring(count) + L" 個符號");
            return; // 成功載入檔案，直接返回
        } else {
            Utils::updateStatus(state, L"標點選單檔案內容過少，使用內建選單");
        }
    }
    
    // 如果檔案載入失敗或內容不足，使用內建選單
    state.punctCandidates = { 
        // 特殊符號
        L"※", L"✓", L"★", L"☆", L"●", L"○",
        
        // 中文標點符號
        L"，", L"。", L"？", L"！", L"：", L"；", 
        
        // 引號和括號
        L"（", L"）", L"「", L"」", L"『", L"』", L"《", L"》", 
        L"〈", L"〉",
        
        // 其他符號
        L"　", L"·", L"－", L"—", L"……", L""", L""", L"'", L"'", 
        L"｜", L"＼", L"／", L"～", L"＿", L"￥", L"％", L"＃", L"＠", 
        L"［", L"］",
        
        // 撲克牌符號
        L"♠", L"♥", L"♣", L"♦"
    };
    
    Utils::updateStatus(state, L"使用內建標點符號選單：" + std::to_wstring(state.punctCandidates.size()) + L" 個符號");
}

void loadUserDict(GlobalState& state) {
    state.wordFreq.clear();
    std::ifstream fin("user_dict.txt");
    if (!fin.is_open()) {
        Utils::updateStatus(state, L"首次使用，將建立用戶字典");
        return;
    }
    
    std::string line;
    int count = 0;
    time_t now = time(nullptr);
    try {
        while (std::getline(fin, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::vector<std::string> parts;
            std::stringstream ss(line);
            std::string part;
            while (std::getline(ss, part, '\t')) {
                parts.push_back(part);
            }
            if (parts.size() >= 2) {
                std::wstring character = Utils::utf8ToWstr(parts[0]);
                int freq = (parts.size() >= 3) ? std::stoi(parts[2]) : 1;
                if (!character.empty()) {
                    state.wordFreq[character] = {freq, now, std::max(3, freq), freq >= 3};
                    count++;
                }
            }
        }
    } catch (...) {}
    fin.close();
    Utils::updateStatus(state, L"重新載入用戶字典：" + std::to_wstring(count) + L" 個記錄");
}

void saveUserDict(const GlobalState& state) {
    try {
        std::ofstream fout("user_dict.txt");
        if (!fout.is_open()) return;
        fout << "# 用戶字典 - 自動生成（已過濾標點符號）" << std::endl;
        fout << "# 格式：詞語<TAB><TAB>使用頻率<TAB>狀態" << std::endl;
        fout << "# 可自行添加修改" << std::endl;
        
        std::vector<std::pair<std::wstring, WordInfo>> freqList;
        for (const auto& pair : state.wordFreq) {
            freqList.push_back(std::make_pair(pair.first, pair.second));
        }
        
        std::sort(freqList.begin(), freqList.end(), 
            [](const std::pair<std::wstring, WordInfo>& a, const std::pair<std::wstring, WordInfo>& b) {
            double scoreA = a.second.frequency * calculateTimeWeight(a.second.lastUsed);
            double scoreB = b.second.frequency * calculateTimeWeight(b.second.lastUsed);
            return scoreA > scoreB;
        });
        
        int maxEntries = std::min(2000, (int)freqList.size());
        for (int i = 0; i < maxEntries; i++) {
            const auto& item = freqList[i];
            std::string status = item.second.isPermanent ? "permanent" : "temp";
            fout << Utils::wstrToUtf8(item.first) << "\t\t" << item.second.frequency << "\t" << status << std::endl;
        }
        fout.close();
    } catch (...) {}
}

bool validateInput(const std::wstring& input) {
    if (input.empty()) return true;
    for (wchar_t ch : input) {
        if (ch != L'u' && ch != L'i' && ch != L'o' && ch != L'j' && ch != L'k' && ch != L'*') {
            return false;
        }
    }
    return true;
}

bool wildcardMatch(const std::wstring& pattern, const std::wstring& text) {
    int pLen = pattern.length();
    int tLen = text.length();
    
    std::vector<std::vector<bool>> dp(tLen + 1, std::vector<bool>(pLen + 1, false));
    
    dp[0][0] = true;
    
    for (int j = 1; j <= pLen; j++) {
        if (pattern[j-1] == L'*') {
            dp[0][j] = dp[0][j-1];
        }
    }
    
    for (int i = 1; i <= tLen; i++) {
        for (int j = 1; j <= pLen; j++) {
            if (pattern[j-1] == L'*') {
                dp[i][j] = dp[i-1][j] || dp[i][j-1];
            } else if (pattern[j-1] == text[i-1]) {
                dp[i][j] = dp[i-1][j-1];
            }
        }
    }
    
    return dp[tLen][pLen];
}

void sortCandidatesBySmartScore(GlobalState& state) {
    std::vector<std::pair<std::wstring, std::wstring>> candidatePairs;
    for (size_t i = 0; i < state.candidates.size(); i++) {
        candidatePairs.push_back(std::make_pair(state.candidates[i], state.candidateCodes[i]));
    }
    
    std::sort(candidatePairs.begin(), candidatePairs.end(), 
        [&state](const std::pair<std::wstring, std::wstring>& a, const std::pair<std::wstring, std::wstring>& b) {
        double scoreA = getWordScore(state, a.first, a.second);
        double scoreB = getWordScore(state, b.first, b.second);
        return scoreA > scoreB;
    });
    
    state.candidates.clear();
    state.candidateCodes.clear();
    for (const auto& pair : candidatePairs) {
        state.candidates.push_back(pair.first);
        state.candidateCodes.push_back(pair.second);
    }
}


// 改進的候選字更新函數
void updateCandidates(GlobalState& state) {
    state.candidates.clear();
    state.candidateCodes.clear();
    state.selected = 0;
    state.currentPage = 0;
    state.inputError = false;
    
    if (state.input.empty()) { 
        state.showCand = false;
        state.isInputting = false;
        if (state.hCandWnd) ShowWindow(state.hCandWnd, SW_HIDE);
        if (state.hInputWnd) ShowWindow(state.hInputWnd, SW_HIDE);
        std::wstring modeText = state.chineseMode ? L"中文筆劃+全形" : L"英文直接+半形";
        Utils::updateStatus(state, modeText + L"模式" + (state.bufferMode ? L" [暫放模式]" : L""));
        return; 
    }
    
    // 使用增強型驗證
    if (!enhancedValidateInput(state.input)) {
        state.inputError = true;
        state.showCand = false;
        // ★ 關鍵修改：保持輸入狀態，不設為false
        state.isInputting = true;  
        
        // 保持字碼輸入視窗顯示
        if (state.hInputWnd) {
            ShowWindow(state.hInputWnd, SW_SHOW);
            InvalidateRect(state.hInputWnd, nullptr, TRUE);
        }
        
        // 隱藏候選字視窗但立即重新定位字碼視窗
        if (state.hCandWnd) {
            ShowWindow(state.hCandWnd, SW_HIDE);
        }
        
        // ★ 新增：強制重新定位字碼視窗
        WindowManager::positionInputWindow(state);
        
        Utils::updateStatus(state, L"字碼過長：建議使用(3+3)搜尋或清除重新輸入");
        if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
        return;
    }
    
    std::wstring filteredInput = filterValidChars(state.input);
    
    if (filteredInput.empty()) {
        state.inputError = true;
        state.showCand = false;
        // ★ 關鍵修改：保持輸入狀態
        state.isInputting = true;
        
        if (state.hInputWnd) {
            ShowWindow(state.hInputWnd, SW_SHOW);
            InvalidateRect(state.hInputWnd, nullptr, TRUE);
        }
        
        if (state.hCandWnd) {
            ShowWindow(state.hCandWnd, SW_HIDE);
        }
        
        // ★ 新增：強制重新定位字碼視窗
        WindowManager::positionInputWindow(state);
        
        Utils::updateStatus(state, L"請輸入有效字碼：uiojk或*");
        if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
        return;
    }
    
    // 候選字查找邏輯（保持原有）
    bool hasWildcard = filteredInput.find(L'*') != std::wstring::npos;
    if (hasWildcard) {
        for (const auto& pair : state.dict) {
            if (wildcardMatch(filteredInput, pair.first)) {
                for (const auto& character : pair.second) {
                    state.candidates.push_back(character);
                    state.candidateCodes.push_back(pair.first);
                }
            }
        }
    } else {
        if (state.dict.count(filteredInput)) {
            for (const auto& character : state.dict[filteredInput]) {
                state.candidates.push_back(character);
                state.candidateCodes.push_back(filteredInput);
            }
        }
        
        // 前綴匹配
        int prefixMatchCount = 0;
        const int MAX_PREFIX_MATCHES = 50;
        for (const auto& pair : state.dict) {
            if (prefixMatchCount >= MAX_PREFIX_MATCHES) break;
            if (pair.first.length() > filteredInput.length() && 
                pair.first.substr(0, filteredInput.length()) == filteredInput) {
                for (const auto& character : pair.second) {
                    if (std::find(state.candidates.begin(), state.candidates.end(), character) == state.candidates.end()) {
                        state.candidates.push_back(character);
                        state.candidateCodes.push_back(pair.first);
                        prefixMatchCount++;
                        if (prefixMatchCount >= MAX_PREFIX_MATCHES) break;
                    }
                }
            }
        }
        
        // 自動(3+3)搜尋
        if (filteredInput.length() > 8 && state.candidates.empty()) {
            std::wstring first3 = filteredInput.substr(0, std::min(3, (int)filteredInput.length()));
            std::wstring last3;
            if (filteredInput.length() >= 6) {
                last3 = filteredInput.substr(filteredInput.length() - 3);
            } else if (filteredInput.length() > 3) {
                last3 = filteredInput.substr(3);
            }
            std::wstring searchPattern = first3 + L"*" + last3;
            for (const auto& pair : state.dict) {
                if (wildcardMatch(searchPattern, pair.first)) {
                    for (const auto& character : pair.second) {
                        state.candidates.push_back(character);
                        state.candidateCodes.push_back(pair.first);
                    }
                }
            }
        }
    }
    
    sortCandidatesBySmartScore(state);
    state.totalPages = (state.candidates.size() + CANDIDATES_PER_PAGE - 1) / CANDIDATES_PER_PAGE;
    state.showCand = !state.candidates.empty();
    // ★ 關鍵修改：無論是否有候選字都保持輸入狀態
    state.isInputting = true;
    
    // ★ 修改：統一使用 WindowManager 來處理視窗定位
    if (state.showCand) {
        // 有候選字時，定位候選字視窗和字碼視窗
        WindowManager::positionWindowsOptimized(state);
    } else {
        // 沒有候選字時，只定位字碼視窗
        WindowManager::positionInputWindow(state);
        if (state.hCandWnd) ShowWindow(state.hCandWnd, SW_HIDE);
    }
    
    std::wstring statusMsg;
    if (state.showCand) {
        std::wstring searchType = hasWildcard ? L"(3+3)模式搜尋" : L"智慧排序搜尋";
        statusMsg = searchType + L"：找到 " + std::to_wstring(state.candidates.size()) + L" 個候選字";
    } else {
        statusMsg = L"輸入中：" + filteredInput + L"（無候選字）";
    }
    
    // 3+3模式建議
    if (filteredInput.length() > 6 && !hasWildcard && state.candidates.empty()) {
        std::wstring first3 = filteredInput.substr(0, 3);
        std::wstring last3;
        if (filteredInput.length() >= 6) {
            last3 = filteredInput.substr(filteredInput.length() - 3);
        } else if (filteredInput.length() > 3) {
            last3 = filteredInput.substr(3);
        }
        if (!last3.empty()) {
            statusMsg += L" | 建議(3+3)：" + first3 + L"*" + last3;
        }
    }
    
    if (state.bufferMode) {
        statusMsg = L"[暫放模式] " + statusMsg;
    }
    
    Utils::updateStatus(state, statusMsg);
    
    if (state.hCandWnd) InvalidateRect(state.hCandWnd, nullptr, TRUE);
    if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
}


void selectCandidate(GlobalState& state, int idx) {
    int actualIndex = state.currentPage * CANDIDATES_PER_PAGE + idx;
    if (actualIndex < 0 || actualIndex >= (int)state.candidates.size()) return;
    std::wstring selected = state.candidates[actualIndex];
    
    if (state.bufferMode) {
        // 暫放模式下：所有選擇的文字（包括標點符號）都插入暫放區
        BufferManager::insertTextAtCursor(state, selected);
        if (!state.showPunctMenu) {
            learnWord(state, selected);
            saveUserDict(state);
        }
        if (state.showPunctMenu) {
            Utils::updateStatus(state, L"已加入標點符號：" + selected + L" (共" + std::to_wstring(state.bufferText.length()) + L"字)");
        } else {
            Utils::updateStatus(state, L"已加入暫放區：" + selected + L" (共" + std::to_wstring(state.bufferText.length()) + L"字)");
        }
    } else {
        // 非暫放模式：直接發送到目標應用程式
        InputHandler::sendTextDirectUnicode(selected);
        if (!state.showPunctMenu) {
            learnWord(state, selected);
            saveUserDict(state);
        }
    }
    
    state.input.clear();
    state.candidates.clear();
    state.candidateCodes.clear();
    state.showCand = false;
    state.isInputting = false;
    state.inputError = false;
    state.showPunctMenu = false;

    // 同時隱藏候選字視窗和字碼視窗
    if (state.hCandWnd) ShowWindow(state.hCandWnd, SW_HIDE);
    if (state.hInputWnd) ShowWindow(state.hInputWnd, SW_HIDE);
    
    if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);	
}

void changePage(GlobalState& state, int direction) {
    if (!state.showCand || state.totalPages <= 1) return;
    if (direction > 0 && state.currentPage < state.totalPages - 1) {
        state.currentPage++;
        state.selected = 0;
    } else if (direction < 0 && state.currentPage > 0) {
        state.currentPage--;
        state.selected = 0;
    }
    Utils::updateStatus(state, L"第" + std::to_wstring(state.currentPage + 1) + L"/" + 
                        std::to_wstring(state.totalPages) + L"頁 共" + 
                        std::to_wstring(state.candidates.size()) + L"個候選字");
    if (state.hCandWnd) InvalidateRect(state.hCandWnd, nullptr, TRUE);
}

void autoApply3Plus3Mode(GlobalState& state) {
    if (state.input.length() > 12) {
        std::wstring first3 = state.input.substr(0, 3);
        std::wstring last3 = state.input.substr(state.input.length() - 3);
        state.input = first3 + L"*" + last3;
        
        Utils::updateStatus(state, L"自動轉換為(3+3)模式：" + state.input);
        updateCandidates(state);
    }
}

void suggest3Plus3Mode(const GlobalState& state) {
    if (state.input.length() > 8) {
        std::wstring first3 = state.input.substr(0, 3);
        std::wstring last3 = state.input.substr(state.input.length() - 3);
        std::wstring suggestion = first3 + L"*" + last3;
        
        Utils::updateStatus(const_cast<GlobalState&>(state), 
                           L"建議(3+3)模式：" + suggestion + L"（可節省輸入時間）");
    }  
}

} // namespace Dictionary