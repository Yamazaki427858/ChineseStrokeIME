// main.cpp 測試版 6.0 - 配色自訂+全功能刷新+候選字字體版
#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>

using namespace std;

// ========== 全域變數 ==========
HWND g_hWnd = NULL;
HWND g_hCandWnd = NULL;
HHOOK g_hKeyboardHook = NULL;
wstring g_input = L"";
vector<wstring> g_candidates;
vector<wstring> g_candidateCodes;
void auto_apply_3plus3_mode();
void suggest_3plus3_mode();
map<wstring, vector<wstring>> g_dict;
map<wstring, vector<wstring>> g_punct;

// 優化：增強的頻率系統
struct WordInfo {
    int frequency;
    time_t lastUsed;
    int tempCount;
    bool isPermanent;
};

map<wstring, WordInfo> g_wordFreq;
map<wstring, vector<wstring>> g_contextLearning;
wstring g_lastSelected = L"";

int g_selected = 0;
int g_currentPage = 0;
int g_totalPages = 0;
const int CANDIDATES_PER_PAGE = 9;
bool g_showCand = false;
bool g_chineseMode = true;
bool g_isInputting = false;
bool g_inputError = false;
bool g_showPunctMenu = false;

// Shift鍵切換模式的狀態追蹤
bool g_shiftPressed = false;
bool g_shiftUsedForCombo = false;
DWORD g_shiftPressTime = 0;

// 介面相關變數 - 新增刷新按鈕
RECT g_closeButtonRect = {0};
RECT g_modeButtonRect = {0};
RECT g_creditsButtonRect = {0};
RECT g_refreshButtonRect = {0};
bool g_closeButtonHover = false;
bool g_modeButtonHover = false;
bool g_creditsButtonHover = false;
bool g_refreshButtonHover = false;
vector<wstring> g_punctCandidates;
bool g_isDragging = false;
POINT g_dragStartPoint = {0};

// 修正：完整的介面設定變數 - 分離主視窗和候選字視窗的配色
COLORREF g_bgColor = RGB(240,240,240);                          // 主視窗背景色
COLORREF g_textColor = RGB(0,0,0);                               // 主視窗文字顏色
COLORREF g_selColor = RGB(0,120,215);                            // 主視窗選擇顏色
COLORREF g_selBgColor = RGB(230,240,250);                        // 主視窗選擇背景色
COLORREF g_errorColor = RGB(220,50,50);                          // 錯誤顏色
COLORREF g_closeButtonColor = RGB(220,50,50);                    // 關閉按鈕顏色
COLORREF g_closeButtonHoverColor = RGB(255,70,70);               // 關閉按鈕懸停顏色
COLORREF g_modeButtonColor = RGB(100,50,200);                    // 模式按鈕顏色
COLORREF g_modeButtonHoverColor = RGB(120,70,220);               // 模式按鈕懸停顏色
COLORREF g_creditsButtonColor = RGB(200,150,50);                 // 製作群按鈕顏色
COLORREF g_creditsButtonHoverColor = RGB(220,170,70);            // 製作群按鈕懸停顏色
COLORREF g_refreshButtonColor = RGB(50,150,50);                  // 刷新按鈕顏色
COLORREF g_refreshButtonHoverColor = RGB(70,170,70);             // 刷新按鈕懸停顏色

// 新增：候選字視窗專用配色變數
COLORREF g_candidateBackgroundColor = RGB(255,255,255);          // 候選字視窗背景色
COLORREF g_candidateTextColor = RGB(0,0,0);                      // 候選字文字顏色
COLORREF g_selectedCandidateBackgroundColor = RGB(230,240,250);  // 選中候選字背景色
COLORREF g_selectedCandidateTextColor = RGB(0,120,215);          // 選中候選字文字顏色

int g_fontSize = 16;                                             // 主視窗字體
wstring g_fontName = L"Microsoft JhengHei";                     // 主視窗字體名稱
int g_candidateFontSize = 18;                                    // 候選字字體大小
wstring g_candidateFontName = L"Microsoft JhengHei";            // 候選字字體名稱
int g_windowWidth = 580;
int g_windowHeight = 70;
int g_candidateWidth = 500;
int g_candidateHeight = 320;

// 狀態信息
wstring g_statusInfo = L"就緒";
int g_dictSize = 0;

// ========== 工具函數 ==========
wstring utf8_to_wstr(const string& str) {
    if (str.empty()) return wstring();
    int sz = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
    if (sz <= 0) return wstring();
    wstring res(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &res[0], sz);
    return res;
}

string wstr_to_utf8(const wstring& ws) {
    if (ws.empty()) return string();
    int sz = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    if (sz <= 0) return string();
    string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), &res[0], sz, nullptr, nullptr);
    return res;
}

void update_status(const wstring& msg) {
    g_statusInfo = msg;
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
}

// 顏色解析函數
COLORREF parse_color_from_string(const string& colorStr) {
    if (colorStr.empty()) return RGB(0,0,0);
    
    // 支援 #RRGGBB 格式
    if (colorStr[0] == '#' && colorStr.length() == 7) {
        unsigned long rgb = strtoul(colorStr.substr(1).c_str(), nullptr, 16);
        return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
    }
    
    // 支援純16進制格式
    if (colorStr.length() == 6) {
        bool isHex = true;
        for (char c : colorStr) {
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                isHex = false;
                break;
            }
        }
        if (isHex) {
            unsigned long rgb = strtoul(colorStr.c_str(), nullptr, 16);
            return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
        }
    }
    
    return RGB(0,0,0);
}

// 新增：標點符號判斷函數
bool is_punctuation(const wstring& word) {
    if (word.empty()) return false;
    
    // 定義完整的標點符號字元集（包含中英文標點符號）
    wstring punctuations = L"，。？！：；（）「」【】『』《》〈〉、·－—……""''｜＼／～＿￥％＃＠｛｝"
                          L",.?!:;()[]{}\"'<>/\\-_@#$%^&*+=|`~"
                          L"　"; // 包含全形空格
    
    // 檢查字詞是否只包含標點符號或空白字元
    for (wchar_t ch : word) {
        // 如果是空白字元
        if (ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r' || ch == L'　') {
            continue;
        }
        // 如果不是標點符號，則返回false
        if (punctuations.find(ch) == wstring::npos) {
            return false;
        }
    }
    return true;  // 全部都是標點符號或空白字元
}

// 修正：完整的配置載入函數，支援所有候選字配色
void load_interface_config() {
    ifstream fin("interface_config.ini");
    if (!fin.is_open()) {
        update_status(L"使用預設介面配色");
        return;
    }
    
    string line;
    string currentSection = "";
    
    while (getline(fin, line)) {
        // 移除前後空白
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        if (line.empty() || line[0] == '#' || line[0] == ';') continue;
        
        // 檢查是否為section
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        size_t eq = line.find('=');
        if (eq != string::npos) {
            string key = line.substr(0, eq);
            string value = line.substr(eq + 1);
            
            // 移除空白
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            
            // 處理Colors section
            if (currentSection == "Colors") {
                if (key == "background_color") {
                    g_bgColor = parse_color_from_string(value);
                } else if (key == "text_color") {
                    g_textColor = parse_color_from_string(value);
                } else if (key == "selection_color") {
                    g_selColor = parse_color_from_string(value);
                } else if (key == "selection_background_color") {
                    g_selBgColor = parse_color_from_string(value);
                } else if (key == "error_color") {
                    g_errorColor = parse_color_from_string(value);
                } else if (key == "close_button_color") {
                    g_closeButtonColor = parse_color_from_string(value);
                } else if (key == "close_button_hover_color") {
                    g_closeButtonHoverColor = parse_color_from_string(value);
                } else if (key == "mode_button_color") {
                    g_modeButtonColor = parse_color_from_string(value);
                } else if (key == "mode_button_hover_color") {
                    g_modeButtonHoverColor = parse_color_from_string(value);
                } else if (key == "credits_button_color") {
                    g_creditsButtonColor = parse_color_from_string(value);
                } else if (key == "credits_button_hover_color") {
                    g_creditsButtonHoverColor = parse_color_from_string(value);
                } else if (key == "refresh_button_color") {
                    g_refreshButtonColor = parse_color_from_string(value);
                } else if (key == "refresh_button_hover_color") {
                    g_refreshButtonHoverColor = parse_color_from_string(value);
                }
                // 新增：候選字專用配色處理
                else if (key == "candidate_background_color") {
                    g_candidateBackgroundColor = parse_color_from_string(value);
                } else if (key == "candidate_text_color") {
                    g_candidateTextColor = parse_color_from_string(value);
                } else if (key == "selected_candidate_background_color") {
                    g_selectedCandidateBackgroundColor = parse_color_from_string(value);
                } else if (key == "selected_candidate_text_color") {
                    g_selectedCandidateTextColor = parse_color_from_string(value);
                }
            }
            // 處理Font section
            else if (currentSection == "Font") {
                if (key == "font_size") {
                    try {
                        g_fontSize = stoi(value);
                    } catch (...) {}
                } else if (key == "font_name") {
                    g_fontName = utf8_to_wstr(value);
                } else if (key == "candidate_font_size") {
                    try {
                        g_candidateFontSize = stoi(value);
                    } catch (...) {}
                } else if (key == "candidate_font_name") {
                    g_candidateFontName = utf8_to_wstr(value);
                }
            }
            // 處理Window section
            else if (currentSection == "Window") {
                if (key == "window_width") {
                    try {
                        g_windowWidth = stoi(value);
                    } catch (...) {}
                } else if (key == "window_height") {
                    try {
                        g_windowHeight = stoi(value);
                    } catch (...) {}
                } else if (key == "candidate_width") {
                    try {
                        g_candidateWidth = stoi(value);
                    } catch (...) {}
                } else if (key == "candidate_height") {
                    try {
                        g_candidateHeight = stoi(value);
                    } catch (...) {}
                }
            }
        }
    }
    fin.close();
    
    // 重新設置視窗大小
    if (g_hWnd) {
        SetWindowPos(g_hWnd, NULL, 0, 0, g_windowWidth, g_windowHeight, SWP_NOMOVE | SWP_NOZORDER);
    }
    if (g_hCandWnd) {
        SetWindowPos(g_hCandWnd, NULL, 0, 0, g_candidateWidth, g_candidateHeight, SWP_NOMOVE | SWP_NOZORDER);
    }
    
    update_status(L"重新載入介面配置（含完整候選字配色）");
}

double calculate_time_weight(time_t lastUsed) {
    time_t now = time(nullptr);
    double daysDiff = difftime(now, lastUsed) / (24 * 3600);
    if (daysDiff <= 1) return 1.0;
    if (daysDiff <= 7) return 0.8;
    if (daysDiff <= 30) return 0.6;
    if (daysDiff <= 90) return 0.4;
    return 0.2;
}

// 修正：添加標點符號過濾的學習函數
void learn_word(const wstring& word) {
    // 新增：過濾標點符號和空白字元
    if (is_punctuation(word)) {
        return;  // 不學習標點符號
    }
    
    // 新增：過濾空白或過短的詞語
    if (word.empty() || word.length() == 0) {
        return;
    }
    
    time_t now = time(nullptr);
    if (g_wordFreq.find(word) == g_wordFreq.end()) {
        g_wordFreq[word] = {1, now, 1, false};
        update_status(L"學習新詞：" + word + L"（暫存）");
    } else {
        WordInfo& info = g_wordFreq[word];
        info.frequency++;
        info.lastUsed = now;
        if (!info.isPermanent) {
            info.tempCount++;
            if (info.tempCount >= 3) {
                info.isPermanent = true;
                update_status(L"詞語加入永久詞庫：" + word);
            } else {
                update_status(L"詞語學習中：" + word + L"（" + to_wstring(info.tempCount) + L"/3）");
            }
        }
    }
    if (!g_lastSelected.empty() && g_lastSelected != word) {
        g_contextLearning[g_lastSelected].push_back(word);
        if (g_contextLearning[g_lastSelected].size() > 10) {
            g_contextLearning[g_lastSelected].erase(g_contextLearning[g_lastSelected].begin());
        }
    }
    g_lastSelected = word;
}

double get_word_score(const wstring& word, const wstring& code) {
    double score = (10.0 - code.length()) * 2.0;
    if (g_wordFreq.find(word) != g_wordFreq.end()) {
        const WordInfo& info = g_wordFreq[word];
        double freqScore = info.frequency * 1.0;
        double timeWeight = calculate_time_weight(info.lastUsed);
        double permanentBonus = info.isPermanent ? 5.0 : 0.0;
        score += (freqScore * timeWeight) + permanentBonus;
    }
    if (!g_lastSelected.empty() && g_contextLearning.find(g_lastSelected) != g_contextLearning.end()) {
        auto& context = g_contextLearning[g_lastSelected];
        if (find(context.begin(), context.end(), word) != context.end()) {
            score += 3.0;
        }
    }
    return score;
}

void send_text_direct_unicode(const wstring& text) {
    if (text.empty()) return;
    
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
        
        Sleep(5);
    }
}

void load_dict(const char* fname, map<wstring, vector<wstring>>& dict) {
    dict.clear();  // 清空現有字典
    ifstream fin(fname);
    if (!fin.is_open()) {
        update_status(L"無法載入字典檔案，使用內建字典");
        dict[L"u"] = {L"一"};
        dict[L"i"] = {L"丨"};
        dict[L"o"] = {L"丿"};
        dict[L"j"] = {L"丶"};
        dict[L"k"] = {L"乙"};
        g_dictSize = 5;
        return;
    }
    string line;
    int count = 0;
    while (getline(fin, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t tab = line.find('\t');
        if (tab == string::npos) continue;
        wstring key = utf8_to_wstr(line.substr(tab+1));
        wstring val = utf8_to_wstr(line.substr(0, tab));
        if (!key.empty() && !val.empty()) {
            dict[key].push_back(val);
            count++;
        }
    }
    fin.close();
    g_dictSize = count;
    update_status(L"重新載入中文字典：" + to_wstring(count) + L" 個字");
}

void load_punctuator() {
    g_punct[L","] = {L"，", L","};
    g_punct[L"."] = {L"。", L"."};
    g_punct[L"?"] = {L"？", L"?"};
    g_punct[L"!"] = {L"！", L"!"};
    g_punct[L":"] = {L"：", L":"};
    g_punct[L";"] = {L"；", L";"};
    g_punct[L"("] = {L"（", L"("};
    g_punct[L")"] = {L"）", L")"};
    g_punct[L"["] = {L"「", L"【", L"［", L"["};
    g_punct[L"]"] = {L"」", L"】", L"］", L"]"};
    g_punct[L"{"] = {L"『", L"{"};
    g_punct[L"}"] = {L"』", L"}"};
    g_punct[L" "] = {L" "};
    g_punct[L"<"] = {L"《", L"<"};
    g_punct[L">"] = {L"》", L">"};
    g_punct[L"/"] = {L"／", L"/"};
    g_punct[L"'"] = {L"、", L"'"};
    // 新增：完整的中文標點符號映射
    g_punct[L"-"] = {L"－", L"-"};
	g_punct[L"_"] = {L"＿", L"_"};
    g_punct[L"="] = {L"＝", L"="};
    g_punct[L"\\"] = {L"＼", L"\\"};
    g_punct[L"|"] = {L"｜", L"|"}; 
    g_punct[L"~"] = {L"～", L"~"}; 
    g_punct[L"`"] = {L"`", L"`"};
    g_punct[L"^"] = {L"⌃", L"^"};
    g_punct[L"&"] = {L"＆", L"&"}; 
    g_punct[L"*"] = {L"＊", L"*"}; 
    g_punct[L"+"] = {L"＋", L"+"};
    g_punct[L"#"] = {L"＃", L"#"};
    g_punct[L"@"] = {L"＠", L"@"};   
    g_punct[L"$"] = {L"＄", L"$"}; 
    g_punct[L"%"] = {L"％", L"%"};
	g_punct[L"\""] = {L"＂", L"\""};
}

void load_punct_menu_from_file() {
    g_punctCandidates.clear();  // 清空現有標點符號列表
    ifstream fin("punct_menu.txt");
    if (!fin.is_open()) {
        update_status(L"無法載入標點符號選單檔案，使用內建選單");
        // 使用內建標點符號列表作為備份
        g_punctCandidates = { 
            L"※", L"✓", L"，", L"。", L"？", L"！", L"：", L"；", 
            L"（", L"）", L"「", L"」", L"【", L"】", L"『", L"』", 
            L"《", L"》", L"〈", L"〉", L"、", L"·", L"－", L"—", 
            L"……", L""", L""", L"'", L"'", L"｜", L"＼", L"／", 
            L"～", L"＿", L"￥", L"％", L"＃", L"＠", L"｛", L"｝" 
        };
        return;
    }
    
    string line;
    int count = 0;
    while (getline(fin, line)) {
        // 移除前後空白
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);
        
        // 跳過空行和註釋行
        if (line.empty() || line[0] == '#') continue;
        
        // 支援 TAB 分隔的格式：標點符號<TAB>描述
        size_t tab = line.find('\t');
        string punctStr;
        if (tab != string::npos) {
            punctStr = line.substr(0, tab);  // 只取標點符號部分
        } else {
            punctStr = line;  // 整行都是標點符號
        }
        
        // 轉換為寬字符並添加到列表
        wstring punct = utf8_to_wstr(punctStr);
        if (!punct.empty()) {
            g_punctCandidates.push_back(punct);
            count++;
        }
    }
    fin.close();
    
    // 如果載入的標點符號太少，使用內建備份
    if (count < 5) {
        update_status(L"標點符號選單檔案內容過少，使用內建選單");
        g_punctCandidates = { 
            L"※", L"✓", L"，", L"。", L"？", L"！", L"：", L"；", 
            L"（", L"）", L"「", L"」", L"【", L"】", L"『", L"』", 
            L"《", L"》", L"〈", L"〉", L"、", L"·", L"－", L"—", 
            L"……", L""", L""", L"'", L"'", L"｜", L"＼", L"／", 
            L"～", L"＿", L"￥", L"％", L"＃", L"＠", L"｛", L"｝" 
        };
    } else {
        update_status(L"載入標點符號選單：" + to_wstring(count) + L" 個符號");
    }
}

void load_punct_menu() {
     load_punct_menu_from_file();
}

void load_userdict() {
    g_wordFreq.clear();  // 清空現有用戶字典
    ifstream fin("user_dict.txt");
    if (!fin.is_open()) {
        update_status(L"首次使用，將建立用戶字典");
        return;
    }
    string line;
    int count = 0;
    time_t now = time(nullptr);
    try {
        while (getline(fin, line)) {
            if (line.empty() || line[0] == '#') continue;
            vector<string> parts;
            stringstream ss(line);
            string part;
            while (getline(ss, part, '\t')) {
                parts.push_back(part);
            }
            if (parts.size() >= 2) {
                wstring character = utf8_to_wstr(parts[0]);
                int freq = (parts.size() >= 3) ? stoi(parts[2]) : 1;
                if (!character.empty()) {
                    g_wordFreq[character] = {freq, now, max(3, freq), freq >= 3};
                    count++;
                }
            }
        }
    } catch (...) {}
    fin.close();
    update_status(L"重新載入用戶字典：" + to_wstring(count) + L" 個記錄");
}

void save_userdict() {
    try {
        ofstream fout("user_dict.txt");
        if (!fout.is_open()) return;
        fout << "# 用戶字典 - 自動生成（已過濾標點符號）" << endl;
        fout << "# 格式：詞語<TAB><TAB>使用頻率<TAB>狀態" << endl;
        fout << "# 可自行添加修改" << endl;
        vector<pair<wstring, WordInfo>> freqList;
        for (const auto& pair : g_wordFreq) {
            freqList.push_back(make_pair(pair.first, pair.second));
        }
        sort(freqList.begin(), freqList.end(), [](const pair<wstring, WordInfo>& a, const pair<wstring, WordInfo>& b) {
            double scoreA = a.second.frequency * calculate_time_weight(a.second.lastUsed);
            double scoreB = b.second.frequency * calculate_time_weight(b.second.lastUsed);
            return scoreA > scoreB;
        });
        int maxEntries = min(2000, (int)freqList.size());
        for (int i = 0; i < maxEntries; i++) {
            const auto& item = freqList[i];
            string status = item.second.isPermanent ? "permanent" : "temp";
            fout << wstr_to_utf8(item.first) << "\t\t" << item.second.frequency << "\t" << status << endl;
        }
        fout.close();
        
    } catch (...) {}
}

void load_config() {
    load_interface_config();
    load_punct_menu();
    update_status(L"載入設定檔完成");
}

bool validate_input(const wstring& input) {
    if (input.empty()) return true;
    for (wchar_t ch : input) {
        if (ch != L'u' && ch != L'i' && ch != L'o' && ch != L'j' && ch != L'k' && ch != L'*') {
            return false;
        }
    }
    return true;
}

void toggle_input_mode() {
    g_chineseMode = !g_chineseMode;
    g_input.clear();
    g_candidates.clear();
    g_candidateCodes.clear();
    g_showCand = false;
    g_isInputting = false;
    g_inputError = false;
    g_showPunctMenu = false;
    if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
    wstring modeMsg = g_chineseMode ? L"中文+全形" : L"英文+半形";
    update_status(L"Shift切換到" + modeMsg + L"模式");
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
}

LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        DWORD key = pKeyboard->vkCode;

        if (key == VK_SHIFT || key == VK_LSHIFT || key == VK_RSHIFT) {
            if (wParam == WM_KEYDOWN) {
                if (!g_shiftPressed) {
                    g_shiftPressed = true;
                    g_shiftUsedForCombo = false;
                    g_shiftPressTime = GetTickCount();
                }
            } else if (wParam == WM_KEYUP) {
                if (g_shiftPressed) {
                    g_shiftPressed = false;
                    DWORD pressDuration = GetTickCount() - g_shiftPressTime;
                    if (!g_shiftUsedForCombo && pressDuration < 500 && pressDuration > 30) {
                        toggle_input_mode();
                    }
                    // 新增：Shift鍵釋放時重置組合鍵狀態
                    g_shiftUsedForCombo = false;
                }
            }
            return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
        }
        
        if (wParam == WM_KEYDOWN) {
            if (g_shiftPressed && key != VK_SHIFT && key != VK_LSHIFT && key != VK_RSHIFT) {
                g_shiftUsedForCombo = true;
            }

            bool isStrokeKey = (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L' || key == 'P' ||
                                key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || 
                                key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0);

            // 修正：重新設計標點符號鍵判斷邏輯
            bool isPunctKey = (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 ||
                   key == VK_OEM_1 || key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 ||
                   key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||  // 新增
                   (key == '1' && g_shiftPressed) || (key == '2' && g_shiftPressed) || 
                   (key == '3' && g_shiftPressed) || (key == '4' && g_shiftPressed) || 
                   (key == '5' && g_shiftPressed) || (key == '6' && g_shiftPressed) || 
                   (key == '7' && g_shiftPressed) || (key == '8' && g_shiftPressed) || 
                   (key == '9' && g_shiftPressed) || (key == '0' && g_shiftPressed));
            
            bool isFunctionKey = ((g_isInputting || g_showPunctMenu) && 
                      (key == VK_SPACE || key == VK_BACK || key == VK_ESCAPE ||
                       key == VK_UP || key == VK_DOWN || key == VK_TAB ||  // 新增Tab鍵
                       (key >= '1' && key <= '9')));

            
            if (g_chineseMode) {
                if (isStrokeKey || isPunctKey || (key == VK_SPACE && g_isInputting) || isFunctionKey) {
                    PostMessage(g_hWnd, WM_USER+100, key, 0);
                    return 1;
                }
            } else {
                if (isPunctKey || isFunctionKey) {
                     PostMessage(g_hWnd, WM_USER+100, key, 0);
                     return 1;
                }
            }
        }
        // 新增：非Shift相關按鍵釋放時，重置組合鍵狀態（避免狀態殘留）
        else if (wParam == WM_KEYUP) {
            if (key != VK_SHIFT && key != VK_LSHIFT && key != VK_RSHIFT && !g_shiftPressed) {
                g_shiftUsedForCombo = false;
            }
        }
    }
    
    return CallNextHookEx(g_hKeyboardHook, nCode, wParam, lParam);
}

bool g6_wildcard_match(const wstring& pattern, const wstring& text) {
    int pLen = pattern.length();
    int tLen = text.length();
    
    // 動態規劃表
    vector<vector<bool>> dp(tLen + 1, vector<bool>(pLen + 1, false));
    
    // 空模式匹配空字符串
    dp[0][0] = true;
    
    // 處理模式開頭的 * 號
    for (int j = 1; j <= pLen; j++) {
        if (pattern[j-1] == L'*') {
            dp[0][j] = dp[0][j-1];
        }
    }
    
    // 填充DP表
    for (int i = 1; i <= tLen; i++) {
        for (int j = 1; j <= pLen; j++) {
            if (pattern[j-1] == L'*') {
                // * 可以匹配0個或多個字符
                dp[i][j] = dp[i-1][j] || dp[i][j-1];
            } else if (pattern[j-1] == text[i-1]) {
                // 字符完全匹配
                dp[i][j] = dp[i-1][j-1];
            }
            // 其他情況保持false
        }
    }
    
    return dp[tLen][pLen];
}


void sort_candidates_by_smart_score() {
    vector<pair<wstring, wstring>> candidatePairs;
    for (size_t i = 0; i < g_candidates.size(); i++) {
        candidatePairs.push_back(make_pair(g_candidates[i], g_candidateCodes[i]));
    }
    sort(candidatePairs.begin(), candidatePairs.end(), [](const pair<wstring, wstring>& a, const pair<wstring, wstring>& b) {
        double scoreA = get_word_score(a.first, a.second);
        double scoreB = get_word_score(b.first, b.second);
        return scoreA > scoreB;
    });
    g_candidates.clear();
    g_candidateCodes.clear();
    for (const auto& pair : candidatePairs) {
        g_candidates.push_back(pair.first);
        g_candidateCodes.push_back(pair.second);
    }
}

void update_candidates() {
    g_candidates.clear();
    g_candidateCodes.clear();
    g_selected = 0;
    g_currentPage = 0;
    g_inputError = false;
    if (g_input.empty()) { 
        g_showCand = false;
        g_isInputting = false;
        if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE); 
        wstring modeText = g_chineseMode ? L"中文筆劃+全形" : L"英文直接+半形";
        update_status(modeText + L"模式");
        return; 
    }
    if (!validate_input(g_input)) {
        g_inputError = true;
        g_showCand = false;
        g_isInputting = true;
        if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
        update_status(L"輸入不當：請使用uiojk或*");
        if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
        return;
    }
    
    bool hasWildcard = g_input.find(L'*') != wstring::npos;
    if (hasWildcard) {
        for (const auto& pair : g_dict) {
            if (g6_wildcard_match(g_input, pair.first)) {
                for (const auto& character : pair.second) {
                    g_candidates.push_back(character);
                    g_candidateCodes.push_back(pair.first);
                }
            }
        }
    } else {
        if (g_dict.count(g_input)) {
            for (const auto& character : g_dict[g_input]) {
                g_candidates.push_back(character);
                g_candidateCodes.push_back(g_input);
            }
        }
        for (const auto& pair : g_dict) {
            if (pair.first.length() > g_input.length() && pair.first.substr(0, g_input.length()) == g_input) {
                for (const auto& character : pair.second) {
                    if (find(g_candidates.begin(), g_candidates.end(), character) == g_candidates.end()) {
                        g_candidates.push_back(character);
                        g_candidateCodes.push_back(pair.first);
                    }
                }
            }
        }
    }
    sort_candidates_by_smart_score();
    g_totalPages = (g_candidates.size() + CANDIDATES_PER_PAGE - 1) / CANDIDATES_PER_PAGE;
    g_showCand = !g_candidates.empty();
    g_isInputting = true;
    
    // 修正：狀態顯示邏輯，包含 (3+3) 提示
    wstring statusMsg;
    if (g_showCand && g_hCandWnd) {
        ShowWindow(g_hCandWnd, SW_SHOW);
        wstring searchType = hasWildcard ? L"(3+3)模式搜尋" : L"智慧排序搜尋";
        statusMsg = searchType + L"：找到 " + to_wstring(g_candidates.size()) + L" 個候選字";
    } else {
        if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
        statusMsg = L"輸入中：" + g_input + L"（無候選字）";
    }
    
    // 新增：在狀態信息後添加 (3+3) 提示
    if (g_input.length() > 8 && !hasWildcard) {
        wstring first3 = g_input.substr(0, 3);
        wstring last3 = g_input.substr(g_input.length() - 3);
        wstring suggestion = first3 + L"*" + last3;
        statusMsg += L" | 💡建議：" + suggestion;
    }
    
    update_status(statusMsg);
    
    if (g_hCandWnd) InvalidateRect(g_hCandWnd, nullptr, TRUE);
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
}


// 新增：自動應用(3+3)模式
void auto_apply_3plus3_mode() {
    if (g_input.length() > 12) {
        wstring first3 = g_input.substr(0, 3);
        wstring last3 = g_input.substr(g_input.length() - 3);
        g_input = first3 + L"*" + last3;
        
        update_status(L"自動轉換為(3+3)模式：" + g_input);
        update_candidates();  // 遞歸調用處理(3+3)模式
    }
}


void show_punct_menu() {
    g_showPunctMenu = true;
    g_candidates = g_punctCandidates;
    g_candidateCodes.clear();
    for (size_t i = 0; i < g_candidates.size(); i++) {
        g_candidateCodes.push_back(L"P");
    }
    g_selected = 0;
    g_currentPage = 0;
    g_totalPages = (g_candidates.size() + CANDIDATES_PER_PAGE - 1) / CANDIDATES_PER_PAGE;
    g_showCand = true;
    if (g_hCandWnd) {
        ShowWindow(g_hCandWnd, SW_SHOW);
        InvalidateRect(g_hCandWnd, nullptr, TRUE);
    }
    update_status(L"全形標點符號選單（按ESC關閉）");
}

void process_stroke(DWORD key) {
    if (!g_chineseMode) return;
    if (key == 'P') {
        show_punct_menu();
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
        g_input += inputChar;
        
        
        update_candidates();
        if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
    }
}

// 新增：(3+3)模式智能提示函數
// 修正：正確的函數結構
void suggest_3plus3_mode() {
    if (g_input.length() > 8) {  // 超過8筆劃啟用提示
        wstring first3 = g_input.substr(0, 3);
        wstring last3 = g_input.substr(g_input.length() - 3);
        wstring suggestion = first3 + L"*" + last3;
        
        update_status(L"💡建議(3+3)模式：" + suggestion + L"（可節省輸入時間）");
    }  
}      




// 修正：移除標點符號學習的標點處理函數
void process_punctuator(DWORD key) {
    wstring punctChar = L"";
    bool isShiftPressed = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    switch (key) {
        // 現有的按鍵映射
        case VK_OEM_COMMA: punctChar = isShiftPressed ? L"<" : L","; break;
        case VK_OEM_PERIOD: punctChar = isShiftPressed ? L">" : L"."; break;
        case VK_OEM_2: punctChar = isShiftPressed ? L"?" : L"/"; break;
        case '1': if (isShiftPressed) punctChar = L"!"; break;
        case VK_OEM_1: if (isShiftPressed) punctChar = L":"; else punctChar = L";"; break;
        case '9': if (isShiftPressed) punctChar = L"("; break;
        case '0': if (isShiftPressed) punctChar = L")"; break;
        case VK_OEM_4: 
            punctChar = isShiftPressed ? L"{" : L"["; 
            break;
        case VK_OEM_6: 
            punctChar = isShiftPressed ? L"}" : L"]"; 
            break;
        case VK_SPACE: punctChar = L" "; break;
        case VK_OEM_7: 
            if (isShiftPressed) {
                punctChar.push_back(L'"');
            } else {
                punctChar = L"'";
            }
            break;
            
        // 修正：減號和下劃線的按鍵映射
        case VK_OEM_MINUS: 
            punctChar = isShiftPressed ? L"_" : L"-";           // Shift+- = 下劃線，- = 減號
            break;
        case VK_OEM_PLUS: 
            punctChar = isShiftPressed ? L"+" : L"=";           // Shift+= = 加號，= = 等號
            break;
            
        // 其他標點符號按鍵映射
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
    
    if (!punctChar.empty() && g_punct.count(punctChar)) {
        vector<wstring> options = g_punct[punctChar];
        if (!options.empty()) {
            wstring selectedPunct;
            if (punctChar == L" ") {
                selectedPunct = L" ";
            } else if (punctChar == L"'") {
                selectedPunct = g_chineseMode ? L"、" : L"'";
			}
             else {
                if (punctChar[0] == L'"' || punctChar == L"[" || punctChar == L"]" || punctChar == L"{" || punctChar == L"}") {
                    selectedPunct = g_chineseMode ? options[0] : options.back();
                } else {
                    selectedPunct = g_chineseMode ? options[0] : (options.size() > 1 ? options[1] : options[0]);
                }
            }
            send_text_direct_unicode(selectedPunct);
            update_status(L"輸入標點：" + selectedPunct);
        }
    }
}



void change_page(int direction) {
    if (!g_showCand || g_totalPages <= 1) return;
    if (direction > 0 && g_currentPage < g_totalPages - 1) {
        g_currentPage++;
        g_selected = 0;
    } else if (direction < 0 && g_currentPage > 0) {
        g_currentPage--;
        g_selected = 0;
    }
    update_status(L"第" + to_wstring(g_currentPage + 1) + L"/" + to_wstring(g_totalPages) + L"頁 共" + to_wstring(g_candidates.size()) + L"個候選字");
    if (g_hCandWnd) InvalidateRect(g_hCandWnd, nullptr, TRUE);
}

// 修正：避免標點符號選單項目被學習
void select_candidate(int idx) {
    int actualIndex = g_currentPage * CANDIDATES_PER_PAGE + idx;
    if (actualIndex < 0 || actualIndex >= (int)g_candidates.size()) return;
    wstring selected = g_candidates[actualIndex];
    send_text_direct_unicode(selected);
    
    // 修正：只學習非標點符號選單的詞語，learn_word函數內部會再次過濾標點符號
    if (!g_showPunctMenu) {
        learn_word(selected);
        save_userdict();
    }
    
    g_input.clear();
    g_candidates.clear();
    g_candidateCodes.clear();
    g_showCand = false;
    g_isInputting = false;
    g_inputError = false;
    g_showPunctMenu = false;
    if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE);
    if (g_hWnd) InvalidateRect(g_hWnd, nullptr, TRUE);
}

void draw_close_button(HDC hdc, RECT windowRect) {
    int buttonSize = 18;
    g_closeButtonRect.left = windowRect.right - buttonSize - 3;
    g_closeButtonRect.top = windowRect.top + 3;
    g_closeButtonRect.right = g_closeButtonRect.left + buttonSize;
    g_closeButtonRect.bottom = g_closeButtonRect.top + buttonSize;
    COLORREF buttonColor = g_closeButtonHover ? g_closeButtonHoverColor : g_closeButtonColor;
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &g_closeButtonRect, hBrush);
    DeleteObject(hBrush);
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255,255,255));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    int margin = 3;
    MoveToEx(hdc, g_closeButtonRect.left + margin, g_closeButtonRect.top + margin, NULL);
    LineTo(hdc, g_closeButtonRect.right - margin, g_closeButtonRect.bottom - margin);
    MoveToEx(hdc, g_closeButtonRect.right - margin, g_closeButtonRect.top + margin, NULL);
    LineTo(hdc, g_closeButtonRect.left + margin, g_closeButtonRect.bottom - margin);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
}

void draw_mode_button(HDC hdc, RECT windowRect) {
    int buttonSize = 18;
    g_modeButtonRect.left = windowRect.right - buttonSize - 25;
    g_modeButtonRect.top = windowRect.top + 3;
    g_modeButtonRect.right = g_modeButtonRect.left + buttonSize;
    g_modeButtonRect.bottom = g_modeButtonRect.top + buttonSize;
    COLORREF buttonColor = g_modeButtonHover ? g_modeButtonHoverColor : g_modeButtonColor;
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &g_modeButtonRect, hBrush);
    DeleteObject(hBrush);
    HFONT hSmallFont = CreateFontW(10,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
    SetTextColor(hdc, RGB(255,255,255));
    SetBkMode(hdc, TRANSPARENT);
    wstring buttonText = g_chineseMode ? L"中" : L"英";
    TextOutW(hdc, g_modeButtonRect.left + 4, g_modeButtonRect.top + 2, buttonText.c_str(), (int)buttonText.size());
    SelectObject(hdc, hOldFont);
    DeleteObject(hSmallFont);
}

void draw_credits_button(HDC hdc, RECT windowRect) {
    int buttonSize = 18;
    g_creditsButtonRect.left = windowRect.right - buttonSize - 47;
    g_creditsButtonRect.top = windowRect.top + 3;
    g_creditsButtonRect.right = g_creditsButtonRect.left + buttonSize;
    g_creditsButtonRect.bottom = g_creditsButtonRect.top + buttonSize;
    COLORREF buttonColor = g_creditsButtonHover ? g_creditsButtonHoverColor : g_creditsButtonColor;
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &g_creditsButtonRect, hBrush);
    DeleteObject(hBrush);
    HFONT hSmallFont = CreateFontW(10,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
    SetTextColor(hdc, RGB(255,255,255));
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, g_creditsButtonRect.left + 6, g_creditsButtonRect.top + 2, L"？", 1);
    SelectObject(hdc, hOldFont);
    DeleteObject(hSmallFont);
}

void draw_refresh_button(HDC hdc, RECT windowRect) {
    int buttonSize = 18;
    g_refreshButtonRect.left = windowRect.right - buttonSize - 69;
    g_refreshButtonRect.top = windowRect.top + 3;
    g_refreshButtonRect.right = g_refreshButtonRect.left + buttonSize;
    g_refreshButtonRect.bottom = g_refreshButtonRect.top + buttonSize;
    COLORREF buttonColor = g_refreshButtonHover ? g_refreshButtonHoverColor : g_refreshButtonColor;
    HBRUSH hBrush = CreateSolidBrush(buttonColor);
    FillRect(hdc, &g_refreshButtonRect, hBrush);
    DeleteObject(hBrush);
    HFONT hSmallFont = CreateFontW(10,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,L"Arial");
    HFONT hOldFont = (HFONT)SelectObject(hdc, hSmallFont);
    SetTextColor(hdc, RGB(255,255,255));
    SetBkMode(hdc, TRANSPARENT);
    TextOutW(hdc, g_refreshButtonRect.left + 5, g_refreshButtonRect.top + 2, L"⟳", 1);
    SelectObject(hdc, hOldFont);
    DeleteObject(hSmallFont);
}

void draw_main(HWND hwnd, HDC hdc) {
    RECT rc; GetClientRect(hwnd, &rc);
    SetBkMode(hdc, TRANSPARENT);
    HBRUSH hBg = CreateSolidBrush(g_bgColor);
    FillRect(hdc, &rc, hBg);
    DeleteObject(hBg);
    draw_close_button(hdc, rc);
    draw_mode_button(hdc, rc);
    draw_credits_button(hdc, rc);
    draw_refresh_button(hdc, rc);
    HFONT hFont = CreateFontW(g_fontSize,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,g_fontName.c_str());
    HFONT hOld = (HFONT)SelectObject(hdc, hFont);
    SetTextColor(hdc, g_chineseMode ? RGB(0,150,0) : RGB(0,100,200));
    wstring mode = g_chineseMode ? L"中文+全形" : L"英文+半形";
    TextOutW(hdc, 10, 5, mode.c_str(), (int)mode.size());
    SetTextColor(hdc, g_inputError ? g_errorColor : g_textColor);
    wstring display;
    if (g_showPunctMenu) {
        display = L"標點符號選單（P鍵開啟）";
    } else if (g_input.empty()) {
        display = g_chineseMode ? L"中文筆劃輸入法（UIOJKL）P=標點，Shift=切換" : L"英文直接輸入，Shift=切換";
    } else if (g_inputError) {
        display = L"輸入不當：" + g_input;
    } else {
        display = g_input;
    }
    TextOutW(hdc, 100, 5, display.c_str(), (int)display.size());
    SetTextColor(hdc, RGB(100, 100, 100));
    TextOutW(hdc, 10, 25, g_statusInfo.c_str(), (int)g_statusInfo.size());
    wstring info = L"測試版 6.0 | 記憶詞庫：" + to_wstring(g_wordFreq.size()) + L" | 字典：" + to_wstring(g_dictSize);
    TextOutW(hdc, 10, 45, info.c_str(), (int)info.size());
    SelectObject(hdc, hOld); DeleteObject(hFont);
}

// 修正：完全重寫候選字繪製函數，使用專用配色變數
void draw_cand(HWND hwnd, HDC hdc) {
    if (g_candidates.empty()) return;
    
    RECT rc; 
    GetClientRect(hwnd, &rc);
    SetBkMode(hdc, TRANSPARENT);
    
    // 繪製邊框，使用空刷子避免覆蓋背景
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(180,180,180));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
    Rectangle(hdc, 0, 0, rc.right, rc.bottom);
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);
    
    // 使用候選字專用字體
    HFONT hFont = CreateFontW(
        g_candidateFontSize,
        0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY,DEFAULT_PITCH|FF_DONTCARE,
        g_candidateFontName.c_str()
    );
    
    HFONT hOld = (HFONT)SelectObject(hdc, hFont);
    
    int lineHeight = g_candidateFontSize + 6;
    int startIndex = g_currentPage * CANDIDATES_PER_PAGE;
    int endIndex = min(startIndex + CANDIDATES_PER_PAGE, (int)g_candidates.size());
    
    for (int i = 0; i < endIndex - startIndex; ++i) {
        int actualIndex = startIndex + i;
        
        // 修正：使用候選字專用文字顏色
        if (i == g_selected) {
            RECT bgRect = {8, 8 + i * lineHeight, rc.right - 8, 8 + (i + 1) * lineHeight};
            HBRUSH hBrush = CreateSolidBrush(g_selectedCandidateBackgroundColor);
            FillRect(hdc, &bgRect, hBrush);
            DeleteObject(hBrush);
            SetTextColor(hdc, g_selectedCandidateTextColor);
        } else {
            SetTextColor(hdc, g_candidateTextColor);
        }
        
        wstring txt;
        if (g_showPunctMenu) {
            txt = to_wstring(i+1) + L". " + g_candidates[actualIndex];
        } else {
            wstring codeInfo = L" [" + g_candidateCodes[actualIndex] + L"]";
            wstring detailInfo = L"";
            if (g_wordFreq.find(g_candidates[actualIndex]) != g_wordFreq.end()) {
                const WordInfo& info = g_wordFreq[g_candidates[actualIndex]];
                double score = get_word_score(g_candidates[actualIndex], g_candidateCodes[actualIndex]);
                detailInfo = L"(" + to_wstring(info.frequency) + L"," + to_wstring((int)score) + L")";
                if (!info.isPermanent) detailInfo += L"[暫]";
            }
            txt = to_wstring(i+1) + L". " + g_candidates[actualIndex] + codeInfo + detailInfo;
        }
        TextOutW(hdc, 15, 10 + i * lineHeight, txt.c_str(), (int)txt.size());
    }
    
    if (g_totalPages > 1) {
        SetTextColor(hdc, RGB(150, 150, 150));
        wstring pageInfo = L"↑↓翻頁 " + to_wstring(g_currentPage + 1) + L"/" + to_wstring(g_totalPages);
        TextOutW(hdc, 15, 10 + CANDIDATES_PER_PAGE * lineHeight, pageInfo.c_str(), (int)pageInfo.size());
    }
    
    SelectObject(hdc, hOld); 
    DeleteObject(hFont);
}

bool is_point_in_close_button(int x, int y) { return (x >= g_closeButtonRect.left && x <= g_closeButtonRect.right && y >= g_closeButtonRect.top && y <= g_closeButtonRect.bottom); }
bool is_point_in_mode_button(int x, int y) { return (x >= g_modeButtonRect.left && x <= g_modeButtonRect.right && y >= g_modeButtonRect.top && y <= g_modeButtonRect.bottom); }
bool is_point_in_credits_button(int x, int y) { return (x >= g_creditsButtonRect.left && x <= g_creditsButtonRect.right && y >= g_creditsButtonRect.top && y <= g_creditsButtonRect.bottom); }
bool is_point_in_refresh_button(int x, int y) { return (x >= g_refreshButtonRect.left && x <= g_refreshButtonRect.right && y >= g_refreshButtonRect.top && y <= g_refreshButtonRect.bottom); }

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_DESTROY:
        if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); g_hKeyboardHook = NULL; }
        save_userdict();
        PostQuitMessage(0); 
        return 0;
        
    case WM_USER+100: {
        DWORD key = (DWORD)wp;
        
        if (g_chineseMode && (key == 'U' || key == 'I' || key == 'O' || key == 'J' || key == 'K' || key == 'L' || key == 'P' || key == VK_NUMPAD7 || key == VK_NUMPAD8 || key == VK_NUMPAD9 || key == VK_NUMPAD4 || key == VK_NUMPAD5 || key == VK_NUMPAD0)) { 
            process_stroke(key); 
            return 0; 
        }
        
        // 修正：添加完整的標點符號鍵檢查
        if (key == VK_OEM_COMMA || key == VK_OEM_PERIOD || key == VK_OEM_2 || key == VK_OEM_1 || 
            key == VK_OEM_4 || key == VK_OEM_6 || key == VK_OEM_7 || key == VK_SPACE ||
            key == VK_OEM_MINUS || key == VK_OEM_PLUS || key == VK_OEM_5 || key == VK_OEM_3 ||
            (key == '1' && (GetKeyState(VK_SHIFT) & 0x8000)) || 
            (key == '2' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
            (key == '3' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
            (key == '4' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
            (key == '5' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
            (key == '6' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
            (key == '7' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
            (key == '8' && (GetKeyState(VK_SHIFT) & 0x8000)) ||
            (key == '9' && (GetKeyState(VK_SHIFT) & 0x8000)) || 
            (key == '0' && (GetKeyState(VK_SHIFT) & 0x8000))) { 
            process_punctuator(key); 
            return 0; 
        }
        
        if (key == VK_DOWN) { change_page(1); return 0; }
        if (key == VK_UP) { change_page(-1); return 0; }
        if (key >= '1' && key <= '9') { select_candidate(key - '1'); return 0; }
        if (key == VK_BACK) { if (!g_input.empty()) { g_input.pop_back(); update_candidates(); InvalidateRect(hwnd, nullptr, TRUE); } return 0; }
        if (key == VK_SPACE) { select_candidate(0); return 0; }
        if (key == VK_ESCAPE) { g_input.clear(); g_candidates.clear(); g_candidateCodes.clear(); g_showCand = false; g_isInputting = false; g_inputError = false; g_showPunctMenu = false; if (g_hCandWnd) ShowWindow(g_hCandWnd, SW_HIDE); update_status(L"已清除輸入"); InvalidateRect(hwnd, nullptr, TRUE); return 0; }
        
        break; 
    }
    
    case WM_PAINT: { 
        PAINTSTRUCT ps; 
        HDC hdc = BeginPaint(hwnd, &ps); 
        draw_main(hwnd, hdc); 
        EndPaint(hwnd, &ps); 
        return 0; 
    }
    
    case WM_LBUTTONDOWN: {
        int x = LOWORD(lp); int y = HIWORD(lp);
        if (is_point_in_close_button(x, y)) { 
            if (MessageBoxW(hwnd, L"確定要關閉中文筆劃輸入法嗎？", L"確認關閉", MB_YESNO | MB_ICONQUESTION) == IDYES) { 
                PostMessage(hwnd, WM_CLOSE, 0, 0); 
            } 
            return 0; 
        }
        if (is_point_in_mode_button(x, y)) { toggle_input_mode(); return 0; }
        if (is_point_in_credits_button(x, y)) { 
            MessageBoxW(hwnd, L"中文筆劃輸入法 - 測試版 6.0 \n\n編寫員：Perplexity.ai\n測試員：山崎大叔（人類）\n\n本次新增功能：\n• 通配符參考G6(3+3)模式\n• 配置刷新按鈕(⟳)\n• 新增punct_menu.txt自訂P鍵標點符號選單\n• 支援interface_config.ini介面配置\n• Colors - 顏色配置\n• Font - 主視窗字體+候選字字體\n• Window - 視窗大小設定 \n\n感謝您的使用！", L"製作人員", MB_OK | MB_ICONINFORMATION); 
            return 0; 
        }
        if (is_point_in_refresh_button(x, y)) { 
            load_interface_config();
            load_dict("Zi-Ma-Biao.txt", g_dict);
            load_punct_menu_from_file();
            load_userdict();
            update_candidates();
            InvalidateRect(hwnd, nullptr, TRUE); 
            if (g_hCandWnd) InvalidateRect(g_hCandWnd, nullptr, TRUE);
            return 0; 
        }
        g_isDragging = true; 
        SetCapture(hwnd); 
        g_dragStartPoint.x = x; 
        g_dragStartPoint.y = y; 
        return 0;
    }
    
    case WM_MOUSEMOVE: {
        if (g_isDragging) {
            POINT pt; GetCursorPos(&pt);
            RECT mainRect; GetWindowRect(hwnd, &mainRect);
            int offsetX = g_dragStartPoint.x; int offsetY = g_dragStartPoint.y;
            int newX = pt.x - offsetX; int newY = pt.y - offsetY;
            HDWP hdwp = BeginDeferWindowPos(2);
            if (hdwp) {
                RECT candRect; GetWindowRect(g_hCandWnd, &candRect);
                int candOffsetX = mainRect.left - candRect.left; int candOffsetY = mainRect.top - candRect.top;
                hdwp = DeferWindowPos(hdwp, hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
                if (g_hCandWnd) { 
                    hdwp = DeferWindowPos(hdwp, g_hCandWnd, NULL, newX - candOffsetX, newY - candOffsetY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE); 
                }
                EndDeferWindowPos(hdwp);
            }
        }
        int x = LOWORD(lp); int y = HIWORD(lp);
        bool wasCloseHover = g_closeButtonHover; 
        bool wasModeHover = g_modeButtonHover; 
        bool wasCreditsHover = g_creditsButtonHover; 
        bool wasRefreshHover = g_refreshButtonHover;
        
        g_closeButtonHover = is_point_in_close_button(x, y); 
        g_modeButtonHover = is_point_in_mode_button(x, y); 
        g_creditsButtonHover = is_point_in_credits_button(x, y); 
        g_refreshButtonHover = is_point_in_refresh_button(x, y);
        
        if (wasCloseHover != g_closeButtonHover || wasModeHover != g_modeButtonHover || wasCreditsHover != g_creditsButtonHover || wasRefreshHover != g_refreshButtonHover) { 
            InvalidateRect(hwnd, nullptr, TRUE); 
        }
        return 0;
    }
    
    case WM_LBUTTONUP: { 
        if (g_isDragging) { 
            g_isDragging = false; 
            ReleaseCapture(); 
            return 0; 
        } 
        break; 
    }
    
    }  // switch 結束
    
    return DefWindowProc(hwnd, msg, wp, lp);
}


// 修正：候選字視窗程序，實現自訂背景繪製
LRESULT CALLBACK CandProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_ERASEBKGND: {
        // 處理自訂背景繪製
        HDC hdc = (HDC)wp;
        RECT rc;
        GetClientRect(hwnd, &rc);
        
        // 使用候選字專用背景色
        HBRUSH hBg = CreateSolidBrush(g_candidateBackgroundColor);
        FillRect(hdc, &rc, hBg);
        DeleteObject(hBg);
        
        return 1;  // 返回1表示背景已處理
    }
    case WM_PAINT: { 
        PAINTSTRUCT ps; 
        HDC hdc = BeginPaint(hwnd, &ps); 
        draw_cand(hwnd, hdc); 
        EndPaint(hwnd, &ps); 
        return 0; 
    }
    case WM_LBUTTONDOWN: { 
        int y = HIWORD(lp); 
        int lineHeight = g_candidateFontSize + 6;
        int idx = (y - 10) / lineHeight; 
        if (idx >= 0 && idx < CANDIDATES_PER_PAGE) { 
            select_candidate(idx); 
        } 
        return 0; 
    }
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    try {
        load_config();
        load_dict("Zi-Ma-Biao.txt", g_dict);
        load_punctuator();
        load_userdict();
        WNDCLASSW wc = {0};
        wc.lpfnWndProc = WndProc; wc.hInstance = hI; wc.lpszClassName = L"IME_MAIN"; 
        wc.hbrBackground = CreateSolidBrush(g_bgColor); 
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassW(&wc)) { MessageBoxW(NULL, L"無法註冊主視窗類別", L"錯誤", MB_OK | MB_ICONERROR); return 1; }
        g_hWnd = CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW, L"IME_MAIN", L"中文筆劃輸入法", WS_POPUP|WS_BORDER, 100, 100, g_windowWidth, g_windowHeight, NULL, NULL, hI, NULL);
        if (!g_hWnd) { MessageBoxW(NULL, L"無法創建主視窗", L"錯誤", MB_OK | MB_ICONERROR); return 1; }
        
        // 修正：候選字視窗類別註冊，設定 hbrBackground 為 NULL 以啟用自訂背景
        WNDCLASSW wc2 = {0};
        wc2.lpfnWndProc = CandProc; wc2.hInstance = hI; wc2.lpszClassName = L"IME_CAND"; 
        wc2.hbrBackground = NULL;  // 關鍵修正：設為 NULL 啟用 WM_ERASEBKGND
        wc2.hCursor = LoadCursor(NULL, IDC_ARROW);
        if (!RegisterClassW(&wc2)) { MessageBoxW(NULL, L"無法註冊候選視窗類別", L"錯誤", MB_OK | MB_ICONERROR); return 1; }
        g_hCandWnd = CreateWindowExW(WS_EX_TOPMOST|WS_EX_TOOLWINDOW, L"IME_CAND", L"", WS_POPUP|WS_BORDER, 100, 180, g_candidateWidth, g_candidateHeight, NULL, NULL, hI, NULL);
        if (!g_hCandWnd) { MessageBoxW(NULL, L"無法創建候選視窗", L"錯誤", MB_OK | MB_ICONERROR); return 1; }
        g_hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardHookProc, hI, 0);
        if (!g_hKeyboardHook) { MessageBoxW(NULL, L"無法安裝鍵盤鉤子，可能需要管理員權限", L"警告", MB_OK | MB_ICONWARNING); }
        ShowWindow(g_hWnd, SW_SHOW); 
        UpdateWindow(g_hWnd);
        update_status(L"中文筆劃輸入法已啟動");
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg); 
            DispatchMessage(&msg);
        }
        if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); }
        return (int)msg.wParam;
    } catch (...) {
        MessageBoxW(NULL, L"程式發生未預期的錯誤", L"錯誤", MB_OK | MB_ICONERROR);
        return 1;
    }
}
