山崎大叔不懂編程，全由Claude AI代勞。所以不作定期更新（看心情），有需要可自行下載SourceCode進行開發。


### 目前最新版本： 中文筆劃輸入法 V2.0 ChineseStrokeIME_Ver2


20251001：Ver2的暫放模式新增文字選取功能、能夠使用"Ctrl+"快捷鍵，最多保存50步歷史。
20250925：發現Ver2﻿的ESC鍵被過度攔截，目前已修復好了。

預設界面：

![image](https://github.com/yamazaki0688/ChineseStrokeIME/blob/ChineseStrokeIME/Images/Preview/ChineseStrokeIME_Ver2/ChineseStrokeIME_Ver2_1.jpg)

暫放模式：

由於輸入法採用全域鍵盤鉤子技術攔截按鍵輸入，若電腦開啟了防毒軟體的「安全瀏覽」或「網銀防護」等功能，可能導致輸入法在瀏覽器中無法使用、打不出字或被直接屏蔽。
遇到此情況時，可啟用「暫放模式」，讓文字先經由「暫放視窗」輸入，再發送到瀏覽器的文字欄位中。

![image](https://github.com/yamazaki0688/ChineseStrokeIME/blob/ChineseStrokeIME/Images/Preview/ChineseStrokeIME_Ver2/ChineseStrokeIME_Ver2_2.jpg)







## 📖 項目簡介

基於 Windows API、採用 C++14 開發的現代化中文筆劃輸入法。集成智慧學習算法、OptimizedUI 介面設計、暫放模式和多螢幕支援。
**綠色免安裝，無需管理員權限**，為用戶提供流暢便捷的中文輸入體驗。



## 👥 開發團隊

| 角色 | 成員 | 貢獻領域 |
|------|------|----------|
| **主要開發者** | Claude AI | 架構設計、核心演算法、UI/UX 設計 |
| **測試工程師** | 山崎大叔（人類） | 功能測試、使用者體驗優化、問題回報 |
| **技術顧問** | 開源社群 | 程式碼審查、效能優化建議 |


### 重要注意事項
⚠️ **防毒軟體相容性警告**

詳情請看：https://github.com/Yamazaki427858/ChineseStrokeIME/blob/ChineseStrokeIME/Images/Preview/ChineseStrokeIME_Ver2/Virustotal%E6%AA%A2%E6%B8%AC%E5%A0%B1%E5%91%8A/Virustotal%E6%AA%A2%E6%B8%AC%E5%A0%B1%E5%91%8A.md

本輸入法採用全域鍵盤鉤子技術來攔截按鍵輸入，此技術可能會被部分防毒軟體誤判為潛在威脅：

- **常見誤報**：Avast、AVG、Windows Defender 等可能會標記為「鍵盤記錄器」
- **安全說明**：本程式為開源專案，所有程式碼公開透明，不會記錄或傳輸任何按鍵資料
- **建議解決方案**：
  1. **加入白名單**：將 `ChineseStrokeIME.exe` 加入防毒軟體的信任清單
  2. **使用暫放模式**：V2.0 新增的暫放模式可降低防毒軟體干擾
  3. **臨時關閉**：安裝時可暫時關閉即時防護，安裝後重新開啟

- **技術說明**：鍵盤鉤子是 Windows 輸入法的標準技術，Microsoft 輸入法、Google 輸入法等都使用相同技術
- **隱私保護**：程式完全本機運行，不連接網路，不收集任何個人資料


## ✨ V2.0 重大更新

### 🎨 OptimizedUI 現代化介面
- **精簡工具列設計**：採用現代化扁平設計風格
- **智能狀態指示器**：實時顯示輸入模式和系統狀態
- **懸停交互效果**：提供直觀的視覺回饋
- **可拖拽視窗組件**：支援工具列和候選字視窗獨立拖拽

### 📝 創新暫放模式
- **防毒軟體兼容**：避免防毒軟體阻擋文字輸入的問題
- **即時編輯功能**：支援中英文混合輸入和即時編輯
- **智能游標控制**：支援方向鍵、Home/End 等完整編輯操作
- **一鍵發送**：Enter 鍵快速發送暫放內容到目標應用

### 🖥️ 多螢幕智能支援
- **自動螢幕偵測**：智能識別延伸模式和鏡像模式
- **位置記憶系統**：分別記憶不同螢幕模式下的視窗位置
- **安全位置恢復**：螢幕配置變更時自動調整到可見位置
- **跨螢幕跟隨**：輸入視窗智能跟隨滑鼠位置

### ⚙️ 系統整合增強
- **系統托盤支援**：可最小化到系統托盤，不佔用工作列空間
- **快捷選單**：右鍵托盤圖示提供完整功能選單
- **自動重啟功能**：支援程式重啟，保留所有設定和學習記錄

## 🎯 核心特色

### 雙模式輸入系統
- **中文筆劃模式**：使用 UIOJK 對應五種基本筆劃（一丨丿丶フ）
- **英文直接模式**：英文字母直接輸出，支援全形/半形切換
- **Shift 快速切換**：單擊 Shift 鍵即可切換模式

### 智能候選字系統
- **最短字碼優先**：優先顯示字碼較短的常用字
- **頻率智能學習**：自動記錄使用習慣，提升常用字排序
- **3+3 萬用搜尋**：支援 `*` 通配符，如 `uoi*jjj` 快速找字
- **上下文學習**：記憶詞組搭配，提供更精準的候選字

### 完整字碼輸入視窗
- **實時字碼顯示**：獨立視窗顯示當前輸入的字碼
- **3+3 模式提示**：自動提示建議的 3+3 搜尋模式
- **錯誤狀態顯示**：清楚標示無效輸入和建議修正
- **跟隨候選字視窗**：與候選字視窗保持最佳相對位置

## 🚀 快速開始

### 系統要求
- **作業系統**：Windows 7 及以上版本
- **編譯環境**：MinGW (支援 C++14) 或 Visual Studio 2017+
- **執行要求**：無需管理員權限，支援便攜式使用


### 編譯安裝

```bash
# 克隆專案
git clone https://github.com/your-username/chinese-stroke-ime.git
cd chinese-stroke-ime

# MinGW 編譯
g++ -std=c++14 -DUNICODE -D_UNICODE -mwindows -O2 \
    main.cpp buffer_manager.cpp config_loader.cpp dictionary.cpp \
    ime_core.cpp input_handler.cpp position_manager.cpp \
    screen_manager.cpp tray_manager.cpp window_manager.cpp \
    -o ChineseStrokeIME.exe -luser32 -lgdi32 -lshell32 -lole32 \
    -static-libgcc -static-libstdc++ -Wl,--subsystem,windows

# 運行程式
./ChineseStrokeIME.exe
```

### 檔案結構

```
├── ChineseStrokeIME.exe        # 主程式
├── Zi-Ma-Biao.txt             # 主字典檔案
├── interface_config.ini        # 介面配置檔案
├── positions.ini               # 位置記憶檔案（自動生成）
├── user_dict.txt              # 用戶學習字典（自動生成）
├── text_buffer.txt            # 暫放內容檔案（自動生成）
└── punct_menu.txt             # 標點符號選單（可選）
```

## 🎛️ 配置系統

### interface_config.ini 完整配置

```ini
; interface_config.ini - 配置文件
; 中文筆劃輸入法 V2.0 介面配置

[Colors]
; 主視窗顏色
background_color=#F0F0F0
text_color=#000000
selection_color=#0078D7
selection_bg_color=#E6F0FA
error_color=#DC3232

; 按鈕顏色
close_button_color=#DC3232
close_button_hover_color=#FF4646
mode_button_color=#6432C8
mode_button_hover_color=#7846DC
credits_button_color=#C89632
credits_button_hover_color=#DC9646
refresh_button_color=#32C832
refresh_button_hover_color=#46DC46

; 候選字視窗顏色
candidate_background_color=#E5FFF8
candidate_text_color=#000000
selected_candidate_bg_color=#F37E7E
selected_candidate_text_color=#01143B

; 字碼輸入視窗顏色
input_background_color=#FFFFFF
input_text_color=#000000
input_error_text_color=#DC3232
input_hint_text_color=#808080
input_border_color=#808080

; 暫放視窗顏色
buffer_background_color=#FFFFFF
buffer_text_color=#000000
buffer_cursor_color=#000000

[Font]
; 主視窗字型
font_size=16
font_name=Microsoft JhengHei

; 候選字視窗字型
candidate_font_size=23
candidate_font_name=Microsoft JhengHei

; 字碼輸入視窗字型
input_font_size=20
input_font_name=Microsoft JhengHei

; 暫放視窗字型
buffer_font_size=20
buffer_font_name=Microsoft JhengHei

[Window]
; 主視窗大小
window_width=580
window_height=70

; 候選字視窗大小 (會根據內容動態調整)
candidate_window_width=300
candidate_window_height=320

; 字碼輸入視窗大小
input_window_width=400
input_window_height=30

[WindowBehavior]
; 前置維護間隔（毫秒）
topmost_check_interval=5000
; 是否使用強制前置模式
force_stay_on_top=1
; 失去焦點後重新前置的延遲（毫秒）
refocus_delay=50
```

## 📋 詳細使用說明

### 基本操作界面

**OptimizedUI 工具列按鈕**：
- **筆劃標識**：顯示「筆劃」字樣表示輸入法類型
- **模式指示器**：「中」表示中文模式，「EN」表示英文模式
- **狀態燈**：綠色=就緒，橙色=輸入中，紅色=錯誤，黃色=暫放模式
- **選單按鈕**：「☰」提供功能選單
- **暫放按鈕**：「⌘」切換暫放模式
- **恢復跟隨**：「⿻」恢復跟隨滑鼠模式
- **最小化**：「－」最小化到系統托盤
- **關閉按鈕**：「×」安全關閉程式

### 中文筆劃輸入詳解

#### 基本筆劃對應表

| 筆劃類型 | 主鍵盤 | 數字鍵盤 | 筆劃樣式 | 說明 |
|----------|--------|----------|----------|------|
| 橫劃 | U | 7 | 一 | 水平線條 |
| 豎劃 | I | 8 | 丨 | 垂直線條 |
| 撇劃 | O | 9 | 丿 | 左下斜線 |
| 捺劃/點 | J | 4 | 丶 | 右下斜線或點 |
| 折劃 | K | 5 | フ | 轉折線條 |
| 通配符 | L | 0 | * | 萬用字符 |

#### 輸入示例與技巧

**簡單字符輸入**：
```
人 → oj (撇 + 捺)
大 → uoj (橫 + 撇 + 捺)
小 → ujj (橫 + 捺 + 捺)
```

**複雜字符與 3+3 模式**：
```
體 (23筆劃完整編碼)：ikkjkikuuikuiiuuikujou
體 (3+3模式)：ikk*jou
効率提升：約70%輸入時間節省
```

**智能提示系統**：
- 輸入超過 6 個字碼時，系統自動建議 3+3 模式
- 輸入超過 12 個字碼時，自動轉換為 3+3 搜尋
- 狀態列顯示：「建議(3+3)：uoi*jjj（可節省輸入時間）」

### 暫放模式詳細說明

暫放模式是 V2.0 的創新功能，專為解決防毒軟體干擾問題而設計：

#### 啟用暫放模式
1. 點擊工具列「⌘」按鈕
2. 或使用系統托盤選單選擇「開啟暫放模式」
3. 狀態指示燈變為黃色，表示暫放模式已啟動

#### 暫放模式功能
- **混合輸入**：支援中文筆劃和英文直接輸入
- **完整編輯**：支援方向鍵移動、Home/End、刪除等操作
- **即時預覽**：暫放視窗顯示當前編輯內容
- **字數統計**：實時顯示字數和游標位置

#### 暫放內容操作
- **Enter 發送**：將暫放內容發送到目標應用程式
- **儲存功能**：自動儲存暫放內容，程式重啟後恢復
- **時間戳存檔**：可另存為帶時間戳的獨立檔案
- **一鍵清空**：快速清除暫放內容

### 進階功能操作

#### 快捷鍵總覽
| 按鍵 | 功能 | 適用模式 |
|------|------|----------|
| **Shift** | 切換中英文模式 | 全域 |
| **1-9** | 選擇對應候選字 | 中文模式 |
| **↑↓** | 翻頁選字 | 中文模式 |
| **Backspace** | 刪除最後字碼/字符 | 全域 |
| **ESC** | 取消當前輸入 | 全域 |
| **Space** | 選擇第一個候選字 | 中文模式 |
| **Enter** | 智能確認/發送暫放內容 | 全域 |
| **P** | 開啟標點符號選單 | 中文模式 |

#### 系統托盤功能
右鍵托盤圖示提供完整功能選單：
- **顯示/隱藏輸入法**：快速切換顯示狀態
- **模式切換**：中英文模式快速切換
- **標點符號選單**：快速存取標點符號
- **暫放模式控制**：開啟/關閉暫放模式
- **位置重置**：恢復跟隨滑鼠模式
- **重新載入配置**：即時載入配置變更
- **程式重啟**：安全重啟並保留設定
- **關於資訊**：顯示版本和開發資訊

## 🏗️ 技術架構

### 核心架構設計

```
中文筆劃輸入法 V2.0
├── 核心引擎 (ime_core)
│   ├── 全域狀態管理
│   ├── 工具函數庫
│   └── 顏色配置系統
├── 輸入處理系統 (input_handler)
│   ├── 低階鍵盤鉤子
│   ├── 智能按鍵分析
│   ├── 模式切換控制
│   └── Unicode 文字發送
├── 字典管理引擎 (dictionary)
│   ├── 高效字典載入
│   ├── 智能排序算法
│   ├── 學習系統
│   ├── 3+3 搜尋引擎
│   └── 標點符號系統
├── 視窗管理系統 (window_manager)
│   ├── OptimizedUI 介面
│   ├── 傳統介面相容
│   ├── 雙緩衝繪製
│   ├── 拖拽功能
│   └── 字碼輸入視窗
├── 暫放管理模組 (buffer_manager)
│   ├── 暫放模式控制
│   ├── 文字編輯功能
│   ├── 檔案自動儲存
│   └── 游標管理系統
├── 多螢幕支援 (screen_manager)
│   ├── 螢幕模式偵測
│   ├── 解析度自適應
│   └── 安全位置計算
├── 位置記憶系統 (position_manager)
│   ├── 多螢幕位置記憶
│   ├── 用戶自定位置
│   └── 自動位置恢復
├── 配置管理 (config_loader)
│   ├── INI 檔案解析
│   ├── 即時配置載入
│   └── 錯誤容錯處理
└── 系統整合 (tray_manager)
    ├── 系統托盤支援
    ├── 右鍵選單
    └── 程式重啟功能
```

### 關鍵技術特性

#### 記憶體效率最佳化
- **智能字典快取**：按需載入，避免記憶體浪費
- **候選字批量處理**：減少重複排序計算
- **GDI 資源池**：重用繪圖資源，避免記憶體洩漏
- **字串最佳化**：使用 `std::wstring` 原生 Unicode 支援

#### 效能優化策略
- **毫秒級按鍵響應**：優化的鍵盤鉤子處理管線
- **雙緩衝繪製**：消除畫面閃爍，提升視覺體驗
- **惰性評估**：延遲計算候選字排序直到真正需要
- **快速字典查詢**：使用 `std::map` 提供 O(log n) 查詢效率

#### 穩定性保證
- **異常安全設計**：完整的 RAII 資源管理
- **螢幕變更容錯**：自動處理螢幕配置變更
- **程式崩潰恢復**：保護用戶學習資料不丟失
- **向下相容性**：支援 Windows 7+ 所有版本

## 🔧 開發與自訂

### 編譯設定
推薦的編譯器設定：
```bash
# 最佳化設定
-O2 -DNDEBUG -march=native

# Unicode 支援
-DUNICODE -D_UNICODE

# 靜態連結（便攜版）
-static-libgcc -static-libstdc++

# 子系統設定
-Wl,--subsystem,windows
```

### 自訂字典格式
`Zi-Ma-Biao.txt` 字典檔案格式：
```
字符<TAB>字碼
一	u
人	oj
你	ikjj
好	ikouj
```

### 標點符號自訂
建立 `punct_menu.txt` 檔案自訂標點符號選單：
```
。
，
？
！
：
；
「」
『』
```

## 🆕 版本更新記錄

### V2.0.0 (2025)
- ✨ 全新 OptimizedUI 介面設計
- 🆕 創新暫放模式功能
- 🖥️ 完整多螢幕支援
- 🎯 智能位置記憶系統
- 🔧 系統托盤整合
- 📝 字碼輸入視窗
- ⚡ 效能與穩定性大幅提升

### V1.x (舊版)
- 基本筆劃輸入功能
- 字典管理與學習系統
- 簡單介面配色支援

## 🤝 貢獻指南

### 開發環境需求
- MinGW-w64 8.0+ 或 Visual Studio 2017+
- Windows SDK 10.0+
- Git 版本控制系統

### 程式碼規範
- 使用 4 空格縮進
- 函數命名採用 camelCase
- 類別命名採用 PascalCase
- 檔案編碼使用 UTF-8 with BOM
- 註釋使用繁體中文

### 提交規範
遵循 [Conventional Commits](https://www.conventionalcommits.org/) 規範：
```
feat: 新增暫放模式自動儲存功能
fix: 修復多螢幕下位置記憶問題
docs: 更新 README 使用說明
refactor: 重構字典載入邏輯
```


## 🔗 相關資源

- [Windows Input Method Editor (IME) 開發指南](https://docs.microsoft.com/en-us/windows/win32/intl/input-method-editors)
- [Unicode 標準文件](https://unicode.org/standard/standard.html)
- [中文輸入法技術參考](https://www.unicode.org/reports/tr14/)
- [Windows GDI+ 繪圖 API](https://docs.microsoft.com/en-us/windows/win32/gdiplus/-gdiplus-gdi-start)

## 📞 支援與回饋

- **問題回報**：請使用 GitHub Issues
- **功能建議**：歡迎提交 Feature Request
- **技術討論**：參與 GitHub Discussions
- **即時支援**：查看 Wiki 文檔

---

**重要提醒**：
- 建議定期備份 `user_dict.txt` 和 `interface_config.ini` 等個人設定檔
- 配置檔案支援即時修改，變更後使用重新載入功能即可生效
- 暫放模式特別適合在有防毒軟體監控的環境中使用
- 多螢幕使用者請留意螢幕配置變更時的自動位置調整功能

⭐ **如果此專案對您有幫助，請給我們一個星標支持！**


※ README.md內容由Claude AI撰寫
