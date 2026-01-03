// dictionary.cpp - 字典管理實作（修正字碼表持續顯示和3+3提示）
#include "dictionary.h"
#include "dict_updater.h"
#include "buffer_manager.h"
#include "input_handler.h"
#include "window_manager.h"
#include "ime_manager.h"
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
        // 文件不存在，尝试从GitHub自动下载
        Utils::updateStatus(state, L"字碼表檔案不存在，嘗試從GitHub下載...");
        if (state.hWnd) {
            InvalidateRect(state.hWnd, nullptr, TRUE);
            UpdateWindow(state.hWnd);
        }
        
        DictUpdater::DownloadResult result = DictUpdater::updateDictionarySafely();
        
        if (result.status == DictUpdater::DownloadStatus::Success) {
            // 下载成功，重新尝试加载
            fin.close();
            fin.open(filename);
            if (fin.is_open()) {
                Utils::updateStatus(state, L"✓ 成功從GitHub下載字碼表，正在載入...");
                if (state.hWnd) {
                    InvalidateRect(state.hWnd, nullptr, TRUE);
                    UpdateWindow(state.hWnd);
                }
                // 继续下面的加载逻辑
            } else {
                // 下载成功但无法打开文件（不应该发生）
                std::wstring errorMsg = L"下載成功但無法打開檔案";
                Utils::updateStatus(state, L"✗ " + errorMsg);
                if (state.hWnd) {
                    MessageBoxW(state.hWnd, 
                        (L"警告：字碼表檔案異常\n\n" + errorMsg + 
                         L"\n\n建議手動下載 Zi-Ma-Biao.txt 文件。").c_str(),
                        L"字碼表載入失敗", MB_OK | MB_ICONWARNING);
                    InvalidateRect(state.hWnd, nullptr, TRUE);
                    UpdateWindow(state.hWnd);
                }
                state.dict[L"u"] = {L"一"};
                state.dict[L"i"] = {L"丨"};
                state.dict[L"o"] = {L"丿"};
                state.dict[L"j"] = {L"丶"};
                state.dict[L"k"] = {L"乙"};
                state.dictSize = 5;
                return;
            }
        } else {
            // 下载失败
            std::wstring errorMsg = DictUpdater::getStatusMessage(result);
            std::wstring fullErrorMsg = L"✗ 無法下載字碼表：" + errorMsg;
            Utils::updateStatus(state, fullErrorMsg);
            
            // 显示弹窗警告
            if (state.hWnd) {
                std::wstring msgBoxText = L"警告：字碼表檔案缺失且下載失敗\n\n";
                msgBoxText += L"錯誤原因：" + errorMsg + L"\n\n";
                msgBoxText += L"建議：\n";
                msgBoxText += L"1. 檢查網路連接\n";
                msgBoxText += L"2. 手動從GitHub下載 Zi-Ma-Biao.txt\n";
                msgBoxText += L"3. 將文件放在程序目錄下\n\n";
                msgBoxText += L"GitHub地址：\n";
                msgBoxText += L"https://github.com/Yamazaki427858/ChineseStrokeIME";
                
                MessageBoxW(state.hWnd, msgBoxText.c_str(), 
                    L"字碼表缺失警告", MB_OK | MB_ICONWARNING);
                InvalidateRect(state.hWnd, nullptr, TRUE);
                UpdateWindow(state.hWnd);
            }
            
            state.dict[L"u"] = {L"一"};
            state.dict[L"i"] = {L"丨"};
            state.dict[L"o"] = {L"丿"};
            state.dict[L"j"] = {L"丶"};
            state.dict[L"k"] = {L"乙"};
            state.dictSize = 5;
            return;
        }
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
        // 修復：重繪工具列以即時更新狀態指示燈（使用 FALSE 只重繪無效區域，減少閃爍）
        if (state.hWnd) InvalidateRect(state.hWnd, nullptr, FALSE);
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
    
    // 判斷是否為聯想字模式（候選字碼為"聯想"、"常用"或"詞語"）
    bool isPredictionMode = (actualIndex < (int)state.candidateCodes.size() && 
                            (state.candidateCodes[actualIndex] == L"聯想" || 
                             state.candidateCodes[actualIndex] == L"常用" ||
                             state.candidateCodes[actualIndex] == L"詞語"));
    
    if (state.bufferMode) {
        // 暫放模式下：所有選擇的文字（包括標點符號）都插入暫放區
        BufferManager::insertTextAtCursor(state, selected);
        if (!state.showPunctMenu) {
            learnWord(state, selected);
            // 延遲保存用戶字典（使用定時器，避免頻繁寫入文件）
            if (state.hWnd) {
                KillTimer(state.hWnd, 995);  // 先清除舊的定時器（使用995避免衝突）
                SetTimer(state.hWnd, 995, 2000, NULL);  // 2秒後保存
            }
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
            // 延遲保存用戶字典（使用定時器，避免頻繁寫入文件）
            if (state.hWnd) {
                KillTimer(state.hWnd, 995);  // 先清除舊的定時器（使用995避免衝突）
                SetTimer(state.hWnd, 995, 2000, NULL);  // 2秒後保存
            }
        }
    }
    
    // 如果是標點符號選單，直接結束
    if (state.showPunctMenu) {
        state.input.clear();
        state.candidates.clear();
        state.candidateCodes.clear();
        state.showCand = false;
        state.isInputting = false;
        state.inputError = false;
        state.showPunctMenu = false;
        if (state.hCandWnd) ShowWindow(state.hCandWnd, SW_HIDE);
        if (state.hInputWnd) ShowWindow(state.hInputWnd, SW_HIDE);
        IMEManager::restoreWindowsIME();
        // 修復：重繪工具列以即時更新狀態指示燈（使用 FALSE 只重繪無效區域，減少閃爍）
        if (state.hWnd) InvalidateRect(state.hWnd, nullptr, FALSE);
        return;
    }
    
    // 如果是聯想字模式，選擇後繼續顯示新的聯想字
    if (isPredictionMode && state.enableWordPrediction) {
        // 清空輸入，準備顯示新的聯想字
        state.input.clear();
        state.inputError = false;
        
        // 顯示該字的聯想字
        showPredictionsAfterSelection(state, selected);
        return;
    }
    
    // 正常模式：選擇字後，如果啟用聯想字功能，顯示聯想字
    if (state.enableWordPrediction && !Utils::isPunctuation(selected)) {
        state.input.clear();
        state.inputError = false;
        
        // 顯示該字的聯想字
        showPredictionsAfterSelection(state, selected);
        return;
    }
    
    // 不啟用聯想字或選擇標點符號：正常結束輸入
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
    
    // 🔥 恢復 Windows 輸入法狀態（輸入完成後）
    IMEManager::restoreWindowsIME();
    
    // 修復：輸入完成後重繪工具列以即時更新狀態指示燈（使用 FALSE 只重繪無效區域，減少閃爍）
    if (state.hWnd) InvalidateRect(state.hWnd, nullptr, FALSE);	
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

// 從GitHub手動更新字典（直接下載，不檢查更新）
bool updateDictFromGitHub(GlobalState& state, bool showProgress) {
    if (showProgress) {
        Utils::updateStatus(state, L"正在從GitHub下載字碼表...");
        if (state.hWnd) {
            InvalidateRect(state.hWnd, nullptr, TRUE);
            UpdateWindow(state.hWnd);
        }
    }
    
    // 直接下載，不使用 GitHub API 檢查（避免 API 訪問次數限制）
    DictUpdater::DownloadResult downloadResult = DictUpdater::updateDictionarySafely();
    
    if (downloadResult.status == DictUpdater::DownloadStatus::Success) {
        // 下載成功，重新載入字典
        std::ifstream testFile("Zi-Ma-Biao.txt");
        if (testFile.is_open()) {
            testFile.close();
            loadMainDict("Zi-Ma-Biao.txt", state);
            Utils::updateStatus(state, L"✓ 字碼表已更新：" + 
                              std::to_wstring(downloadResult.fileSize) + L" 字節");
            if (state.hWnd) {
                InvalidateRect(state.hWnd, nullptr, TRUE);
                UpdateWindow(state.hWnd);
            }
            return true;
        } else {
            // 文件不存在（不应该发生，因为下载成功了）
            Utils::updateStatus(state, L"✗ 更新成功但檔案未找到");
            if (state.hWnd) {
                MessageBoxW(state.hWnd, 
                    L"錯誤：字碼表更新成功但檔案未找到。\n請重新啟動程序或手動檢查。", 
                    L"更新異常", MB_OK | MB_ICONWARNING);
                InvalidateRect(state.hWnd, nullptr, TRUE);
                UpdateWindow(state.hWnd);
            }
            return false;
        }
    } else {
        // 下載失败，显示详细错误信息
        std::wstring errorMsg = DictUpdater::getStatusMessage(downloadResult);
        Utils::updateStatus(state, L"✗ 下載失敗：" + errorMsg);
        
        if (showProgress && state.hWnd) {
            std::wstring msgBoxText = L"字碼表下載失敗\n\n";
            msgBoxText += L"錯誤原因：" + errorMsg + L"\n\n";
            msgBoxText += L"建議：\n";
            msgBoxText += L"1. 檢查網路連接\n";
            msgBoxText += L"2. 稍後重試\n";
            
            MessageBoxW(state.hWnd, msgBoxText.c_str(), 
                L"下載失敗", MB_OK | MB_ICONWARNING);
            InvalidateRect(state.hWnd, nullptr, TRUE);
            UpdateWindow(state.hWnd);
        }
        return false;
    }
}

// 載入詞語庫文件
void loadWordPhrases(GlobalState& state, const char* filename) {
    state.wordPhrases.clear();
    state.phraseDictSize = 0;
    
    std::ifstream fin(filename, std::ios::in | std::ios::binary);
    if (!fin.is_open()) {
        // 文件不存在，嘗試從GitHub自動下載（靜默下載，不顯示提示）
        const char* downloadUrl = 
            "https://raw.githubusercontent.com/Yamazaki427858/ChineseStrokeIME/ChineseStrokeIME/SourceCode/%E8%81%AF%E6%83%B3%E8%A9%9E%E5%BA%AB/word_phrases.txt";
        
        DictUpdater::DownloadResult downloadResult = DictUpdater::downloadFromGitHub(
            downloadUrl, 
            filename,  // 直接下載到目標文件
            30  // 30秒超時
        );
        
        if (downloadResult.status == DictUpdater::DownloadStatus::Success) {
            // 下載成功，重新嘗試打開文件
            fin.close();
            fin.open(filename, std::ios::in | std::ios::binary);
            if (!fin.is_open()) {
                // 下載成功但無法打開文件（不應該發生）
                return;
            }
        } else {
            // 下載失敗，靜默返回（不顯示錯誤）
            return;
        }
    }
    
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
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        
        // 轉換為寬字符
        try {
            std::wstring phrase = Utils::utf8ToWstr(line);
            // 支持2字以上的詞語（不限制最大長度，但建議不超過10字以保持性能）
            if (phrase.length() >= 2 && phrase.length() <= 10) {
                // 為詞語中的每個字（除了最後一個）建立到下一個字的映射
                // 例如「電腦系統管理」會建立：
                // 「電」 → 「腦」
                // 「腦」 → 「系」
                // 「系」 → 「統」
                // 「統」 → 「管」
                // 「管」 → 「理」
                // 這樣可以支持連續聯想：電→腦→系→統→管→理
                for (size_t i = 0; i < phrase.length() - 1; i++) {
                    std::wstring currentChar = phrase.substr(i, 1);
                    std::wstring nextChar = phrase.substr(i + 1, 1);
                    
                    // 檢查是否已存在
                    bool exists = false;
                    if (state.wordPhrases.find(currentChar) != state.wordPhrases.end()) {
                        const auto& existing = state.wordPhrases[currentChar];
                        if (std::find(existing.begin(), existing.end(), nextChar) != existing.end()) {
                            exists = true;
                        }
                    }
                    if (!exists) {
                        state.wordPhrases[currentChar].push_back(nextChar);
                        count++;
                    }
                }
            }
        } catch (...) {
            // 轉換失敗，跳過這行
            continue;
        }
    }
    
    state.phraseDictSize = count;
    if (count > 0) {
        Utils::updateStatus(state, L"載入詞語庫：" + std::to_wstring(count) + L" 個詞語組合");
    }
}

// 獲取聯想字候選列表
void getWordPredictions(GlobalState& state, const std::wstring& word) {
    state.candidates.clear();
    state.candidateCodes.clear();
    
    if (word.empty()) return;
    
    // 0. 從詞語庫中獲取聯想字（最高優先級，如果詞語庫存在）
    if (state.phraseDictSize > 0 && state.wordPhrases.find(word) != state.wordPhrases.end()) {
        const auto& phrases = state.wordPhrases.at(word);
        for (const auto& phraseChar : phrases) {
            if (std::find(state.candidates.begin(), state.candidates.end(), phraseChar) == state.candidates.end()) {
                state.candidates.push_back(phraseChar);
                // 查找該字的字碼
                std::wstring code = L"";
                for (const auto& pair : state.dict) {
                    if (std::find(pair.second.begin(), pair.second.end(), phraseChar) != pair.second.end()) {
                        code = pair.first;
                        break;
                    }
                }
                state.candidateCodes.push_back(code.empty() ? L"詞語" : code);
            }
        }
    }
    
    // 1. 從上下文學習中獲取聯想字（優先級次高）
    if (state.contextLearning.find(word) != state.contextLearning.end()) {
        const auto& contextWords = state.contextLearning.at(word);
        for (const auto& contextWord : contextWords) {
            if (std::find(state.candidates.begin(), state.candidates.end(), contextWord) == state.candidates.end()) {
                state.candidates.push_back(contextWord);
                // 查找該字的字碼（如果有的話）
                std::wstring code = L"";
                for (const auto& pair : state.dict) {
                    if (std::find(pair.second.begin(), pair.second.end(), contextWord) != pair.second.end()) {
                        code = pair.first;
                        break;
                    }
                }
                state.candidateCodes.push_back(code.empty() ? L"聯想" : code);
            }
        }
    }
    
    // 2. 從字典中查找常見的詞語組合（優化版：支持2字詞和3字詞）
    // 查找以該字開頭的常見詞語
    std::map<std::wstring, int> wordScores;  // 字 -> 分數
    
    for (const auto& pair : state.dict) {
        for (const auto& dictWord : pair.second) {
            // 如果字典中的字以選中的字開頭（形成詞語）
            if (dictWord.length() > 1 && dictWord[0] == word[0]) {
                // 提取後續字（支持2字詞和3字詞）
                for (size_t i = 1; i < dictWord.length() && i <= 2; i++) {
                    std::wstring nextChar = dictWord.substr(i, 1);
                    if (nextChar.length() == 1) {
                        // 計算分數（基於詞頻和詞語長度）
                        int score = 0;
                        if (state.wordFreq.find(dictWord) != state.wordFreq.end()) {
                            score = state.wordFreq.at(dictWord).frequency * 2;  // 詞語頻率加權
                        } else {
                            score = 1;  // 基礎分數
                        }
                        // 2字詞優先於3字詞
                        if (dictWord.length() == 2) {
                            score += 5;
                        }
                        if (wordScores.find(nextChar) == wordScores.end() || wordScores[nextChar] < score) {
                            wordScores[nextChar] = score;
                        }
                    }
                }
            }
            // 如果字典中的字以選中的字結尾（形成詞語）
            if (dictWord.length() > 1 && dictWord.back() == word[0]) {
                // 提取前面的字（支持2字詞和3字詞）
                for (size_t i = 0; i < dictWord.length() - 1 && i < 2; i++) {
                    std::wstring prevChar = dictWord.substr(i, 1);
                    if (prevChar.length() == 1) {
                        int score = 0;
                        if (state.wordFreq.find(dictWord) != state.wordFreq.end()) {
                            score = state.wordFreq.at(dictWord).frequency * 2;  // 詞語頻率加權
                        } else {
                            score = 1;  // 基礎分數
                        }
                        // 2字詞優先於3字詞
                        if (dictWord.length() == 2) {
                            score += 5;
                        }
                        if (wordScores.find(prevChar) == wordScores.end() || wordScores[prevChar] < score) {
                            wordScores[prevChar] = score;
                        }
                    }
                }
            }
        }
    }
    
    // 按分數排序並添加到候選列表
    std::vector<std::pair<std::wstring, int>> sortedWords;
    for (const auto& pair : wordScores) {
        if (std::find(state.candidates.begin(), state.candidates.end(), pair.first) == state.candidates.end()) {
            sortedWords.push_back(std::make_pair(pair.first, pair.second));
        }
    }
    std::sort(sortedWords.begin(), sortedWords.end(), 
        [](const std::pair<std::wstring, int>& a, const std::pair<std::wstring, int>& b) {
            return a.second > b.second;
        });
    
    // 限制聯想字數量（最多20個）
    size_t maxPredictions = 20;
    for (size_t i = 0; i < sortedWords.size() && state.candidates.size() < maxPredictions; i++) {
        state.candidates.push_back(sortedWords[i].first);
        // 查找該字的字碼
        std::wstring code = L"";
        for (const auto& pair : state.dict) {
            if (std::find(pair.second.begin(), pair.second.end(), sortedWords[i].first) != pair.second.end()) {
                code = pair.first;
                break;
            }
        }
        state.candidateCodes.push_back(code.empty() ? L"聯想" : code);
    }
    
    // 3. 如果候選字太少，從字典中隨機選擇一些常見字
    if (state.candidates.size() < 5) {
        // 選擇詞頻較高的字作為補充
        std::vector<std::pair<std::wstring, int>> freqWords;
        for (const auto& pair : state.wordFreq) {
            if (pair.first.length() == 1 && 
                std::find(state.candidates.begin(), state.candidates.end(), pair.first) == state.candidates.end()) {
                freqWords.push_back(std::make_pair(pair.first, pair.second.frequency));
            }
        }
        std::sort(freqWords.begin(), freqWords.end(), 
            [](const std::pair<std::wstring, int>& a, const std::pair<std::wstring, int>& b) {
                return a.second > b.second;
            });
        
        // maxPredictions 已在上面定義為 size_t，這裡直接使用
        for (size_t i = 0; i < freqWords.size() && state.candidates.size() < maxPredictions; i++) {
            state.candidates.push_back(freqWords[i].first);
            // 查找該字的字碼
            std::wstring code = L"";
            for (const auto& pair : state.dict) {
                if (std::find(pair.second.begin(), pair.second.end(), freqWords[i].first) != pair.second.end()) {
                    code = pair.first;
                    break;
                }
            }
            state.candidateCodes.push_back(code.empty() ? L"常用" : code);
        }
    }
}

// 選擇字後顯示聯想字
void showPredictionsAfterSelection(GlobalState& state, const std::wstring& selected) {
    if (!state.enableWordPrediction) return;
    if (selected.empty()) return;
    if (Utils::isPunctuation(selected)) return;  // 標點符號不觸發聯想
    
    // 獲取聯想字
    getWordPredictions(state, selected);
    
    if (state.candidates.empty()) {
        // 沒有聯想字，正常結束輸入
        state.showCand = false;
        state.isInputting = false;
        if (state.hCandWnd) ShowWindow(state.hCandWnd, SW_HIDE);
        if (state.hInputWnd) ShowWindow(state.hInputWnd, SW_HIDE);
        // 修復：重繪工具列以即時更新狀態指示燈（使用 FALSE 只重繪無效區域，減少閃爍）
        if (state.hWnd) InvalidateRect(state.hWnd, nullptr, FALSE);
        return;
    }
    
    // 有聯想字，顯示候選字視窗
    state.selected = 0;
    state.currentPage = 0;
    state.totalPages = (state.candidates.size() + CANDIDATES_PER_PAGE - 1) / CANDIDATES_PER_PAGE;
    state.showCand = true;
    state.isInputting = true;  // 保持輸入狀態，以便繼續選擇聯想字
    
    // 修復：總是重新定位視窗以確保視窗大小根據聯想字內容自動調整
    // 聯想字的內容可能與之前的候選字不同，需要重新計算視窗大小
    WindowManager::positionWindowsOptimized(state);
    
    // 確保候選字視窗顯示並更新（優化：減少不必要的操作）
    if (state.hCandWnd) {
        if (!IsWindowVisible(state.hCandWnd)) {
            ShowWindow(state.hCandWnd, SW_SHOW);
        }
        // 只更新內容，不強制立即重繪
        InvalidateRect(state.hCandWnd, nullptr, FALSE);
    }
    
    // 確保字碼視窗顯示並更新（優化：減少不必要的操作）
    if (state.hInputWnd) {
        if (!IsWindowVisible(state.hInputWnd)) {
            ShowWindow(state.hInputWnd, SW_SHOW);
        }
        // 只更新內容，不強制立即重繪
        InvalidateRect(state.hInputWnd, nullptr, FALSE);
    }
    
    // 優化：移除頻繁的焦點切換，避免閃爍
    // 只在視窗未顯示時才確保目標應用程式有焦點
    bool needReposition = !IsWindowVisible(state.hCandWnd) || !IsWindowVisible(state.hInputWnd);
    if (needReposition) {
        HWND hForeground = GetForegroundWindow();
        if (hForeground && hForeground != state.hWnd && 
            hForeground != state.hCandWnd && hForeground != state.hInputWnd &&
            hForeground != state.hBufferWnd) {
            // 確保目標視窗保持焦點
            DWORD currentThread = GetCurrentThreadId();
            DWORD targetThread = GetWindowThreadProcessId(hForeground, NULL);
            
            if (currentThread != targetThread) {
                AttachThreadInput(currentThread, targetThread, TRUE);
            }
            
            SetForegroundWindow(hForeground);
            
            if (currentThread != targetThread) {
                AttachThreadInput(currentThread, targetThread, FALSE);
            }
        }
    }
    
    // 優化：移除狀態更新和工具列重繪，減少閃爍
    // Utils::updateStatus(state, L"聯想字：" + selected + L" → " + 
    //                    std::to_wstring(state.candidates.size()) + L" 個候選");
    // if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
}

} // namespace Dictionary