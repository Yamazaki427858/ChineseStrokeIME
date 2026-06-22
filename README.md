## 這頁面是用來當介紹和倉庫


  
山崎大叔不懂編程，全由AI代勞。所以不作定期更新（看心情），有需要可自行下載SourceCode進行開發。
<br>
<br>

✏️好易記網址：https://bit.ly/uiojk 隨時下載
<br>
<br>

### 2026年6月最新版本： 中文筆劃輸入法 V3.3.0
💾下載：https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/releases



👨‍💻SourceCode：https://github.com/Yamazaki427858/ChineseStrokeIME_Releases
<br>
<br>


📮這裡發問題：https://github.com/Yamazaki427858/ChineseStrokeIME/issues



🔎中文筆劃查找器：https://yamazaki427858.github.io/LookUpStrokes/







[![CodeQL](https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/actions/workflows/github-code-scanning/codeql/badge.svg)](https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/actions/workflows/github-code-scanning/codeql)
[![Latest Release](https://img.shields.io/github/v/release/Yamazaki427858/ChineseStrokeIME_Releases)](https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/releases/latest)
[![Downloads](https://img.shields.io/github/downloads/Yamazaki427858/ChineseStrokeIME_Releases/total)](https://github.com/Yamazaki427858/ChineseStrokeIME_Releases/releases)
<br>
<br>




## 📋更新日誌：
20260622：V3.3.0更新，新增了Emoji、刪除指定候選字、多螢幕相容修復。





20260501：V3.2.0更新，新增了筆劃符號顯示、縮小顯示工具列、優化智能聯想引擎。




20260316： 中文筆劃輸入法 V3.1.0發布，強化了**智能聯想引擎**相關功能。



20260103： 中文筆劃輸入法 V3.0發布



20251025：修復Ver2當候選字視窗已經顯示時，按P鍵打開標點選單會導致視窗顯示不完整的問題。



20251001：Ver2的暫放模式新增文字選取功能、能夠使用"Ctrl+"快捷鍵，最多保存50步歷史。



20250925：發現Ver2﻿的ESC鍵被過度攔截，目前已修復好了。

<br>
<br>

[👉 點此查看 中文筆劃輸入法 完整使用說明](http://github.com/Yamazaki427858/ChineseStrokeIME_Releases/blob/main/%E4%B8%AD%E6%96%87%E7%AD%86%E5%8A%83%E8%BC%B8%E5%85%A5%E6%B3%95%20%E4%BD%BF%E7%94%A8%E8%AA%AA%E6%98%8E.md)

<br>
<br>

### V3.3.0 新加入功能：

## 🖥 修復多螢幕相容問題

本版強化延伸桌面、鏡像模式與多螢幕熱插拔下的穩定性：

- **支援 3 螢幕以上**：即時偵測螢幕數量變更（例如 4→3、3→2），工具列若落在已移除的螢幕上，會自動移回安全位置 ✅
- **延伸 ↔ 同步切換**：從多螢幕延伸改為單螢幕同步時，工具列會自動移至主螢幕，避免視窗「消失」在無效座標 🔄
- **依螢幕定位**：候選字、字碼視窗等 UI 改以各螢幕的**工作區**計算位置，減少跨螢幕時偏移或超出可視範圍的情況
- **位置記憶分模式**：延伸模式與鏡像模式分別記住工具列位置，切換顯示配置後較易回到習慣的擺放處

> 若希望在螢幕模式變更時彈出提示，可在 `user\interfaceconfig.ini` 的 `[WindowBehavior]` 將 `showScreenModeNotification=1`。

---

## 🗑 刪除指定候選字

可自行整理**用戶字典**中不需要的候選字：

- 在**字碼候選模式**下，於候選字視窗對目標字**按右鍵**
- 選單顯示「用戶字典：〇〇」，點選 **「刪除此候選字」** 即可從 `user\user_dict.txt` 移除
- 僅對已加入用戶字典的字有效；非用戶自訂候選時，該選項會呈灰色無法使用
- 刪除後候選清單會立即更新，無需重啟程式

適合清除誤加入、重複或不再需要的個人候選，讓選字清單更乾淨。

---

## 😀 Emoji 輸入

新增 Emoji 選單，方便在一般輸入與暫放模式中插入表情符號：

- 按 **`P` 鍵**開啟標點／Emoji 選單，可切換至 **Emoji 分頁**
- 以**分類標籤**瀏覽（如表情、手勢、動物等），支援分頁與滑鼠點選
- 選取後直接插入目前輸入位置；選單保持開啟，可連續選多個，按 **ESC** 關閉
- Emoji 資料存放於 `user\emoji\`；首次使用若本地無資料，程式會嘗試從 GitHub 自動下載
- 亦可從托盤或工具列選單選 **「從 GitHub 更新 Emoji」** 手動更新

---

## 🔔 版本更新提醒：「不再提醒」

啟動約 3 秒後，程式會自動比對 GitHub 上的最新版本。若偵測到新版本，會在**螢幕中央**彈出通知，提供三個選項：

| 按鈕 | 說明 |
|------|------|
| **前往下載** | 開啟 GitHub 專案頁下載新版 |
| **稍後** | 關閉對話框；下次啟動仍會提醒 |
| **不再提醒** | 關閉對話框，之後啟動**不再**自動彈出版本通知 |

選「不再提醒」後，設定會寫入 `user\interfaceconfig.ini`：

```ini
[WindowBehavior]
; 0=仍提醒（預設），1=啟動時不再彈出版本更新通知
suppress_version_update_reminder=1
```

改回 `0` 並重啟輸入法，即可恢復自動提醒。（可能需要刪除舊interfaceconfig.ini才能生效。）

> **補充**：「關於」對話框內的**手動檢查更新**不受此設定影響，隨時可主動查詢是否有新版本。

---

---

# v3.2.0 功能介紹


---

## 📚 大型聯想詞語庫

輸入法內建**詞語聯想**功能：選完一個字後，可根據前文從大型詞語庫中推薦後續用字（例如「香港」→「貿易」「特區」等），與個人學習紀錄（`user\context_learning.txt`）互補，讓連續造句更順手。

**詞語庫檔案**

| 項目 | 說明 |
|------|------|
| 本地路徑 | `system\wordphrases.txt`（與 exe 同層的 `system` 資料夾） |
| 格式 | 每行一個詞語（約 2～10 字，UTF-8）；以 `#` 或 `;` 開頭為註解 |
| 可選權重 | 支援 `詞語<TAB>數字` 微調排序，例如 `愛你一萬年\t20` |
| 快取 | 首次載入或更新 txt 後會產生 `system\wordphrases.cache`（勿手動編輯） |



**下載與更新詞語庫**

- 詞庫目錄（GitHub）：  
  https://github.com/Yamazaki427858/ChineseStrokeIME/tree/ChineseStrokeIME/SourceCode/%E8%81%AF%E6%83%B3%E8%A9%9E%E5%BA%AB
- 請下載其中的 **`word_phrases.txt`**，覆蓋或另存為本地 **`system\wordphrases.txt`**
- 若本地尚無此檔且網路可用，程式啟動時也會嘗試從 GitHub 自動下載（檔名仍存為 `wordphrases.txt`）
- 更新 txt 後**重啟輸入法**即可；若快取與新檔不相容，程式會自動刪除舊 `wordphrases.cache` 並重建

---

## 🚀 詞語庫聯想加速（wordphrases.cache）

針對大型 `wordphrases.txt` 做啟動加速：

- **第一次啟動**：讀取 `system\wordphrases.txt`，建立索引並產生 `system\wordphrases.cache`
- **之後啟動**：優先讀取 `system\wordphrases.cache`，通常比每次重新解析 txt 更快 ⚡
- 快取內建版本辨識；若版本不相容或驗證失敗，會刪除舊快取並從 txt 重建（自我修復）🔧

> 多字前綴索引以「唯一 key 數量」作為安全上限，前綴長度 2～9 字，與連續前文窗口對齊。

---

## 🛡️ 快取更穩：原子寫入避免半截壞檔

重建 `wordphrases.cache` 時採用「原子寫入」：

- 先寫入 `wordphrases.cache.tmp`
- 完整寫入成功後，再一次性替換成正式的 `wordphrases.cache`

即使遇到當機、強制關閉、磁碟空間不足等狀況 💥，也不易留下寫到一半的快取 ✅

---

## 💞 多字前綴聯想

- 除「相鄰字」聯想外，詞語庫建立 **2～9 字前綴 → 剩餘片段** 的索引
- 以 **`lastContext`（最多 9 字滾動前文）** 作為查詢前綴，連續選字可命中較長片語的延伸（例如：愛你 → 一萬年）
- 排序區分「完整前文命中的詞庫接續」與「僅用最後一字的補位」，減少清單被不相干長串佔滿

---

## 🧠 個人學習與顯示（context_learning.txt）

- 同時以 **「完整前文」** 與 **「最後一字」** 查個人學習紀錄
- **第 1 次**出現的關聯即會寫入 `user\context_learning.txt`；次數用於排序權重
- 聯想候選的 **「聯想 / 詞語 / 常用」** 等標籤與排序邏輯已對齊
- 編輯或刪除 `context_learning.txt` 前請**關閉程式**，避免定時寫入覆蓋手動修改

---

## 🖱 聯想字右鍵：刪除／封鎖用對「前文 → 候選」

- 刪除、置頂、封鎖等操作以 **本輪聯想查詢的前文** 作為前字，避免選完字後刪錯關聯

---

## ↩ 選錯聯想可「反學習」

- 在筆劃或聯想選字後，於聯想視窗使用 **Backspace** 可扣回剛學的個人關聯、還原前文狀態（必要時刪上屏一個字）

---

## ⛔ 何時中斷「聯想前文」

以下情況會清空聯想前文（語境斷開）：

- **ESC**（取消輸入）
- **任何標點**
- **切換中／英輸入模式**
- **Enter**（含暫放送出等）
- **退格**在筆劃緩衝已空、且非走「聯想窗撤銷」邏輯時

---

## ⚙ 聯想候選數可在設定檔自訂（預設 100）

```ini
[WindowBehavior]
max_word_predictions=100
```

建議依螢幕與習慣在 **1～1000** 之間調整。

---

## ⌨ 筆劃符號與字碼顯示

可切換輸入框與候選字的顯示方式，方便對照字根或查碼：

**輸入框筆劃符號**（`showStrokeSymbols`，預設開啟）

- **開啟**：字碼視窗顯示 **一丨丿丶フ** 等筆劃符號
- **關閉**：改顯示英文字母 **uiojk**
- 可從工具列右鍵選單切換「筆劃符號：開／關」

**候選字英文字碼**（`showCandidateCode`，預設關閉）

- **開啟**：候選列附帶字根，例如 `3. 十[ui]`
- **關閉**：僅顯示候選字本身

```ini
[WindowBehavior]
showStrokeSymbols=1
showCandidateCode=0
```

---

## 🎹 自訂筆劃五鍵

若預設 **U I O J K**（或數字小鍵盤 **78945**）不符合鍵盤配置，可在設定檔自訂：

- **字母筆劃五鍵**：`enableCustomStrokeKeys=1` 後，以 `strokeKeyU`～`strokeKeyK` 指定（支援 A–Z）
- **數字小鍵盤筆劃五鍵**：`enableCustomNumpadStrokeKeys=1` 後，以 `numpadStrokeKeyU`～`numpadStrokeKeyK` 指定（僅 NumPad0–9）
- 兩組設定**分開生效**，可只改其中一組
- 工具列右鍵選單「自訂筆劃五鍵」可快速開關，變更會寫入 `interfaceconfig.ini`

```ini
[InputSettings]
enableCustomStrokeKeys=0
strokeKeyU=U
strokeKeyI=I
strokeKeyO=O
strokeKeyJ=J
strokeKeyK=K
enableCustomNumpadStrokeKeys=0
numpadStrokeKeyU=NumPad7
numpadStrokeKeyI=NumPad8
numpadStrokeKeyO=NumPad9
numpadStrokeKeyJ=NumPad4
numpadStrokeKeyK=NumPad5
```

---

## 📐 縮小顯示工具列

工具列可切換為**精簡模式**（`toolbarClassicModeBadges=1`）：

- 僅保留 **「劃」** 與 **「E」** 兩個按鈕，占用螢幕空間更小
- 在精簡列上**按右鍵**仍可開啟完整選單（設定、更新、關於等）
- 可從工具列右鍵選單切換「縮小顯示工作列」

適合希望工具列低調、不遮擋工作區的使用者。

---

## ✨ V3.1.0 重大更新

## 🔮 聯想引擎強化
v3.1.0 主要強化了**智能聯想引擎**相關功能，讓「下一個字」的推薦更貼近你的使用習慣：

- **📈 更穩定的聯想排序**：結合字碼表內建的相鄰字對與個人使用記錄，候選順序更合理。
- **⚑⚐ 置頂與鎖定**：可對聯想條目設為「置頂 (pinned)」或「鎖定 (locked)」，常用搭配會固定在前方。
- **📝 右鍵管理**：在候選字／聯想字上按右鍵即可設定置頂、鎖定、刪除此聯想或永不再顯示，操作直覺。
- **💾 個人學習檔**：聯想記錄寫入 `user\context_learning.txt`，重啟後仍會保留你的設定與學習結果。

## 🔧 小問題優化與修復

- **📁 整齊的目錄結構**  
  程式以 exe 所在目錄為根目錄，自動使用並建立 **`system\`**（字碼表、詞語庫等）與 **`user\`**（使用者字典、聯想學習、介面設定、標點選單等）；建議將程式放在英文路徑下，避免中文或特殊字元造成讀寫異常。

- **一丨丿丶フ 筆劃符號開關**  
  可在設定或選單中切換「輸入框顯示筆劃符號（一丨丿丶フ）」或「英文字母（u i o j k）」，依個人習慣選擇。

- **🎯 自訂 3+3 萬用字元按鍵**  
  萬用字元 `*` 的觸發鍵可在 `user\interfaceconfig.ini` 的 `[InputSettings]` 中自訂（如 `wildcardKey1`、`wildcardKey2`），預設為 **L** 與 **NumPad0**，方便不同鍵盤配置。

- **⌨️ 按標點符號清除視窗**  
  輸入標點後會一併清空字碼與候選視窗，避免殘留上一輪的候選，畫面更乾淨。

- 修復了一些Bug


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
3. 「⌘」按鈕變為橙色，表示暫放模式已啟動

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
