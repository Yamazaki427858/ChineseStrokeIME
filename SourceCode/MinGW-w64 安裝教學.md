***

## MinGW-w64 安裝教學

MinGW-w64 是 GCC 編譯器的 Windows 版本，用於編譯 C/C++ 程式。本教學將指導您在 Windows 系統上安裝 MinGW-w64。

### 系統需求

- **作業系統**: Windows 7 及以上版本（建議 Windows 10/11）
- **架構**: 支援 32 位元（x86）與 64 位元（x64）系統
- **磁碟空間**: 至少 500 MB 可用空間

***

## 方法一：使用安裝程式（推薦初學者）

### 步驟 1：下載 MinGW-w64 安裝程式

1. 前往 SourceForge 下載頁面：
   ```
   https://sourceforge.net/projects/mingw-w64/files/
   ```

2. 尋找並下載 `mingw-w64-install.exe` 安裝程式

### 步驟 2：執行安裝程式

1. 雙擊下載的 `mingw-w64-install.exe` 檔案
2. 在安裝設定畫面中，選擇以下選項：

   - **Version**: `8.1.0` 或更新版本（建議選擇最新穩定版）
   - **Architecture**: 
     - `x86_64`（64 位元系統，**推薦**）
     - `i686`（32 位元系統）
   - **Threads**: 
     - `posix`（支援 C++11 多執行緒，**推薦**）
     - `win32`（不支援 C++11 多執行緒）
   - **Exception**: 
     - `seh`（64 位元系統，零開銷異常處理，**推薦**）
     - `dwarf`（僅 32 位元可用）
     - `sjlj`（32/64 位元通用，效能較低）
   - **Build revision**: 選擇最新版本（如 `rev0`）

3. 點擊「Next」開始安裝
4. 安裝路徑建議使用預設值：`C:\Program Files\mingw-w64` 或自訂路徑

### 步驟 3：設定環境變數

#### 方法 A：圖形介面設定

1. 在 Windows 搜尋列輸入「環境變數」，選擇「編輯系統環境變數」
2. 點擊「環境變數」按鈕
3. 在「系統變數」區塊中，找到並選擇 `Path`，點擊「編輯」
4. 點擊「新增」，輸入 MinGW-w64 的 bin 路徑：
   ```
   C:\Program Files\mingw-w64\mingw64\bin
   ```
5. 依序點擊「確定」關閉所有視窗

#### 方法 B：命令列設定

開啟「命令提示字元」（以系統管理員身分執行），輸入：
```cmd
setx PATH "C:\Program Files\mingw-w64\mingw64\bin;%PATH%" /M
```

### 步驟 4：驗證安裝

1. 開啟新的「命令提示字元」視窗（必須是新視窗）
2. 輸入以下指令驗證安裝：
   ```cmd
   gcc --version
   g++ --version
   gdb --version
   ```

3. 如果顯示版本資訊，表示安裝成功！

***

## 方法二：手動解壓縮安裝（進階使用者）

### 步驟 1：下載壓縮檔

1. 前往 MinGW-w64 下載頁面：
   ```
   https://sourceforge.net/projects/mingw-w64/files/
   ```

2. 根據您的系統選擇對應的 `.7z` 壓縮檔：
   
   **Windows 10/11 64 位元系統推薦**：
   ```
   x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z
   ```

### 步驟 2：解壓縮檔案

1. 使用 7-Zip 解壓縮下載的檔案
2. 將解壓縮後的 `mingw64` 資料夾複製到：
   ```
   C:\mingw-w64\mingw64
   ```

### 步驟 3：設定環境變數

將以下路徑加入系統 `PATH`：
```
C:\mingw-w64\mingw64\bin
```

### 步驟 4：驗證安裝

開啟新的命令提示字元，輸入 `gcc --version` 確認安裝成功。

***

## 常見問題排解

### 問題 1：找不到 gcc 指令

**解決方案**：
1. 確認環境變數 `PATH` 是否正確設定
2. 確保開啟的是**新的**命令提示字元視窗
3. 檢查 bin 資料夾路徑是否正確

### 問題 2：安裝程式下載失敗

**解決方案**：
1. 檢查網路連線是否正常
2. 嘗試使用方法二手動下載壓縮檔
3. 使用 VPN 或更換網路環境

### 問題 3：缺少 DLL 檔案

**解決方案**：
確保選擇了 `posix` threads 版本，並且完整複製了所有 MinGW 檔案。

***

## 測試編譯

建立測試檔案 `hello.cpp`：
```cpp
#include <iostream>
int main() {
    std::cout << "Hello, MinGW-w64!" << std::endl;
    return 0;
}
```

編譯並執行：
```cmd
g++ hello.cpp -o hello.exe
hello.exe
```

如果輸出 `Hello, MinGW-w64!`，恭喜您安裝成功！

***

## 用於編譯中文筆劃輸入法

安裝完成後，可以使用以下指令編譯本輸入法：
```cmd
mingw32-make
```

或使用手動編譯指令：
```cmd
g++ -std=c++11 -Wall -O2 -mwindows -DUNICODE -D_UNICODE ^
    main.cpp ime_core.cpp input_handler.cpp dictionary.cpp ^
    dict_updater.cpp buffer_manager.cpp window_manager.cpp ^
    config_loader.cpp screen_manager.cpp position_manager.cpp ^
    tray_manager.cpp ime_manager.cpp ^
    -o ChineseStrokeIME.exe ^
    -static-libgcc -static-libstdc++ ^
    -lgdi32 -luser32 -lkernel32 -lshell32 -lcomctl32 ^
    -limm32 -lwininet -lcrypt32
```

