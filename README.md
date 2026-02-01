## 這頁面是用來當介紹和倉庫
<br>
<br>
山崎大叔不懂編程，全由AI代勞。所以不作定期更新（看心情），有需要可自行下載SourceCode進行開發。
<br>
✏️好易記網址：https://bit.ly/uiojk 隨時下載
<br>
<br>

### 2026年最新版本： 中文筆劃輸入法 V3.0.1
💾下載：https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/releases
<br>
<br>
👨‍💻SourceCode：https://github.com/Yamazaki427858/ChineseStrokeIME_Releases
<br>
<br>
<br>
📮這裡發問題：https://github.com/Yamazaki427858/ChineseStrokeIME/issues
<br>
<br>
🔎中文筆劃查找器：https://yamazaki427858.github.io/LookUpStrokes/
<br>
<br>

[![CodeQL](https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/actions/workflows/github-code-scanning/codeql)
[![Latest Release](https://img.shields.io/github/v/release/Yamazaki427858/ChineseStrokeIME_Releases)](https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/releases/latest)
[![Downloads](https://img.shields.io/github/downloads/Yamazaki427858/ChineseStrokeIME_Releases/total)](https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/releases)
<br>
<br>
## 📋更新日誌：
20260108：V3.0.1更新，解決了在 Windows 檔案總管搜尋欄無法正常輸入中文的問題。
<br>
20260103： 中文筆劃輸入法 V3.0發布
<br>
20251025：修復Ver2當候選字視窗已經顯示時，按P鍵打開標點選單會導致視窗顯示不完整的問題。
<br>
20251001：Ver2的暫放模式新增文字選取功能、能夠使用"Ctrl+"快捷鍵，最多保存50步歷史。
<br>
20250925：發現Ver2﻿的ESC鍵被過度攔截，目前已修復好了。
<br>
<br>

### V3.0 新加入功能：

聯想字：
<br>
![image](https://github.com/Yamazaki427858/ChineseStrokeIME/blob/ChineseStrokeIME/Images/Preview/ChineseStrokeIME_Ver3/%E8%81%AF%E6%83%B3%E5%AD%97.gif)
<br>
<br>
<br>
透明化顯示：
<br>
![image](https://github.com/Yamazaki427858/ChineseStrokeIME/blob/ChineseStrokeIME/Images/Preview/ChineseStrokeIME_Ver3/%E9%80%8F%E6%98%8E%E5%8C%96%E9%A1%AF%E7%A4%BA.gif)
<br>
<br>
<br>
暫放模式加入剪貼簿功能：
<br>
![image](https://github.com/Yamazaki427858/ChineseStrokeIME/blob/ChineseStrokeIME/Images/Preview/ChineseStrokeIME_Ver3/%E6%9A%AB%E6%94%BE%E6%A8%A1%E5%BC%8F%E5%8A%A0%E5%85%A5%E5%89%AA%E8%B2%BC%E7%B0%BF%E5%8A%9F%E8%83%BD.gif)
<br>
<br>
<br>

## 📖 項目簡介

基於 Windows API、採用 C++14 開發的中文筆劃輸入法。
**綠色免安裝，無需管理員權限**，為用戶提供流暢便捷的中文輸入體驗。



## 👥 開發團隊

| 角色 | 成員 | 貢獻領域 |
|------|------|----------|
| **主要開發者** | Cursor AI | 架構設計、核心演算法、UI/UX 設計 |
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
  2. **付錢購買代碼簽署憑證**：我無錢🥲

- **技術說明**：鍵盤鉤子是 Windows 輸入法的標準技術
- **隱私保護**：程式完全本機運行，不連接網路（Zi-Ma-Biao.txt、wordphrases.txt需連接GitHub下載），不收集任何個人資料
<br>
<br>

## ✨ V3.0 重大更新

## 🌐 線上更新字碼表
- 首次啟動時，輸入法可從 GitHub 儲存庫即時下載最新字碼表（只需攜帶 ChineseStrokeIME.exe 即可使用）
- 可手動更新（自定義字碼請先備份）

## 🔔 版本更新檢查
- 啟動時自動檢查 GitHub 上的最新版本（避免頻繁網路請求，預設 24 小時檢查一次）
- 可在「關於」頁面手動檢查更新

## 🔮 智能聯想字功能（測試中）
- 根據前一個選字智能推薦下一個常用字
- 可透過選單隨時開啟或關閉聯想功能（默認關閉，請自行打開）

## 🎨 半透明視窗顯示
- 主視窗與暫放視窗支援半透明顯示
- 可透過配置檔自訂透明度級別（0-255，默認100）
- 選單一鍵開啟或關閉透明效果

## ⏸️ 輸入法暫停功能
- 需要使用其他輸入法時可暫時停用本輸入法
- 暫停時完全釋放鍵盤鉤子資源
- 工具列狀態指示燈顯示為灰色

## 📋 暫放模式新增剪貼簿功能
- 開啟剪貼簿模式後，暫放內容自動複製到系統剪貼簿
- 可直接使用 Ctrl+V 在其他應用程式貼上暫放內容
- 按下「○」按鈕，待綠燈亮起即可使用

## **⚡ 程式碼重構與效能優化**
- 鍵盤鉤子接管所有字母輸入，避免與 Windows 內建輸入法衝突
- 減少冗餘代碼
- 修復了一些Bug

<br>
<br>

## 🎯 核心特色

### 雙模式輸入系統
- **中文筆劃模式**：使用 UIOJK 對應五種基本筆劃（一丨丿丶フ）
- **英文直接模式**：英文字母直接輸出，支援全形/半形切換
- **Shift 快速切換**：單擊 Shift 鍵即可切換中英模式

### 智能候選字系統
- **最短字碼優先**：優先顯示字碼較短的常用字
- **頻率智能學習**：自動記錄使用習慣，提升常用字排序
- **3+3 萬用搜尋**：支援 `*` 通配符，如 `uoi*jjj` 快速找字（L/0鍵）

### 暫放模式
- **備用輸入方案**：在無法正常輸入時使用
- **即時編輯功能**：支援中英文混合輸入和即時編輯
- **一鍵發送**：Enter 鍵快速發送暫放內容到目標應用

## 🚀 快速開始

### 系統要求
- **作業系統**：Windows 7 及以上版本
- **編譯環境**：MinGW (支援 C++14) 或 Visual Studio 2017+
- **執行要求**：無需管理員權限，支援便攜式使用

## 📋 詳細使用說明

### 基本操作界面

**工具列按鈕**：
- **筆劃標識**：顯示「筆劃」字樣表示輸入法類型
- **模式指示器**：「中」表示中文模式，「EN」表示英文模式
- **狀態燈**：綠色=就緒，橙色=輸入中，紅色=錯誤
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
人 → oj (丿 + 丶)
大 → uoj (一 + 丿 + 丶)
小 → oij (丨 + 丿 + 丶)
```

**複雜字符與 3+3 模式**：
```
體 (23筆劃完整編碼)：ikkjkikuuikuiiuuikujou
體 (3+3模式)：ikk*jou
効率提升：約70%輸入時間節省
```

### 暫放模式詳細說明

暫放模式專為無法正常輸入時使用：

#### 啟用暫放模式
1. 點擊工具列「⌘」按鈕
2. 或使用系統托盤選單選擇「開啟暫放模式」
3.「⌘」按鈕變為橙色，表示暫放模式已啟動

#### 暫放模式功能
- **混合輸入**：支援中文筆劃和英文直接輸入
- **完整編輯**：支援方向鍵移動、全選/複制/貼上/刪除等操作
- **剪貼簿模式**：暫放內容自動複製到系統剪貼簿（按下「○」按鈕，待綠燈亮起即可使用）
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
| **Enter** | 發送暫放內容 | 全域 |
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


### 檔案結構

```
├── ChineseStrokeIME.exe       # 主程式
├── Zi-Ma-Biao.txt             # 字碼表檔案（可從 GitHub 下載）
├── interface_config.ini       # 介面配置檔案
├── positions.ini              # 位置記憶檔案（自動生成）
├── user_dict.txt              # 用戶記憶字典（自動生成）
├── text_buffer.txt            # 暫放內容檔案（自動生成）
├── versioncache.txt           # 版本檢查緩存檔案（自動生成）
├── wordphrases.txt            # 聯想字詞庫（可從 GitHub 下載）
└── punct_menu.txt             # 標點符號選單（可選）

```

## 🎛️ 配置系統

### interface_config.ini 完整配置

```ini
; interface_config.ini - 配置文件
; 中文筆劃輸入法 V3.0 介面配置

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

; 候選字視窗大小
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
; 剪貼簿模式開關（0=關閉，1=開啟）
clipboard_mode=0
; 半透明顯示開關（0=關閉，1=開啟）
enable_transparency=0
; 透明度值（0-255，255=完全不透明，0=完全透明）
transparency_alpha=100
; 聯想字功能開關（0=關閉，1=開啟）
enable_word_prediction=0
```

## 🏗️ 技術架構

### 核心架構設計

```
中文筆劃輸入法 V3.0
├── 核心引擎 (ime_core)
│   ├── 全域狀態管理
│   ├── 工具函數庫
│   └── 顏色配置系統
├── 輸入處理系統 (input_handler)
│   ├── 低階鍵盤鉤子
│   ├── 智能按鍵分析
│   ├── 模式切換控制
│   ├── 輸入法暫停管理 🆕
│   └── Unicode 文字發送
├── 字典管理引擎 (dictionary)
│   ├── 高效字典載入
│   ├── 智能排序算法
│   ├── 學習系統
│   ├── 3+3 搜尋引擎
│   ├── 聯想字預測系統 🆕
│   ├── 詞組記憶引擎 🆕
│   └── 標點符號系統
├── 線上更新模組 (dict_updater) 🆕
│   ├── GitHub 字碼表下載
│   ├── 版本檢查系統
│   ├── HTTP/HTTPS 連線管理
│   ├── 檔案完整性驗證
│   └── 自動備份機制
├── 視窗管理系統 (window_manager)
│   ├── OptimizedUI 介面
│   ├── 傳統介面相容
│   ├── 雙緩衝繪製
│   ├── 拖拽功能
│   ├── 半透明效果控制 🆕
│   └── 字碼輸入視窗
├── 暫放管理模組 (buffer_manager)
│   ├── 暫放模式控制
│   ├── 文字編輯功能
│   ├── 檔案自動儲存
│   ├── 剪貼簿即時同步 🆕
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
│   ├── 透明度設定管理 🆕
│   └── 錯誤容錯處理
└── 系統整合 (tray_manager)
    ├── 系統托盤支援
    ├── 右鍵選單
    └── 程式重啟功能

```


## 🔧 開發與自訂


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

### 編譯安裝

```
# 清理舊編譯檔案
mingw32-make clean

# 編譯專案
mingw32-make

# 或直接重新編譯
mingw32-make rebuild

# 編譯並執行
mingw32-make run

# MinGW 編譯（V3.0 版本）
g++ -std=c++11 -Wall -O2 -mwindows -DUNICODE -D_UNICODE \
    main.cpp ime_core.cpp input_handler.cpp dictionary.cpp \
    dict_updater.cpp buffer_manager.cpp window_manager.cpp \
    config_loader.cpp screen_manager.cpp position_manager.cpp \
    tray_manager.cpp ime_manager.cpp \
    -o ChineseStrokeIME.exe \
    -static-libgcc -static-libstdc++ \
    -lgdi32 -luser32 -lkernel32 -lshell32 -lcomctl32 \
    -limm32 -lwininet -lcrypt32 \
    -Wl,--subsystem,windows

# 運行程式
./ChineseStrokeIME.exe
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


※ README.md內容由AI撰寫
