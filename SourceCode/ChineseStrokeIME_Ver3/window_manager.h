// window_manager.h - 視窗管理與繪製 (OptimizedUI支援版)
#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "ime_core.h"

namespace WindowManager {
    // OptimizedUI視窗註冊與建立
    bool registerOptimizedWindowClasses(HINSTANCE hInstance);
    bool createOptimizedWindows(HINSTANCE hInstance, GlobalState& state);
    
    // 視窗程序
    LRESULT CALLBACK CandProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT CALLBACK BufferProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    
    // OptimizedUI視窗程序
    LRESULT CALLBACK OptimizedWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    // OptimizedCandProc 已合併到統一的 CandProc 中
    
	// 字碼視窗相關函數
    void drawInputWindow(HDC hdc, RECT rc, const GlobalState& state);
    void positionInputWindow(GlobalState& state);
    LRESULT CALLBACK InputProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    bool createInputWindow(HINSTANCE hInstance, GlobalState& state);
	
    // 繪製函數
    void drawCandidate(HWND hwnd, HDC hdc, const GlobalState& state);
    void drawBufferWindow(HDC hdc, RECT rc, GlobalState& state);
    
    // OptimizedUI繪製函數
    void drawOptimizedToolbar(HDC hdc, RECT rc, GlobalState& state);
    // 注意：drawOptimizedCandidate 已移除，未使用
    
    // 注意：传统UI按钮绘制函数已移除，现在使用 OptimizedUI
    
    // 按鈕點擊檢測
    // 注意：isPointInCloseButton, isPointInModeButton, isPointInCreditsButton, 
    // isPointInRefreshButton, isPointInBufferButton 已移除（传统UI用）
    bool isPointInSendButton(int x, int y, const GlobalState& state);  // BufferProc中使用
    bool isPointInClearButton(int x, int y, const GlobalState& state);  // BufferProc中使用
    bool isPointInSaveButton(int x, int y, const GlobalState& state);   // BufferProc中使用
	bool isPointInPrevPageButton(int x, int y, const GlobalState& state);  // CandProc中使用
	bool isPointInNextPageButton(int x, int y, const GlobalState& state);  // CandProc中使用
    
    // OptimizedUI按鈕點擊檢測
    bool isPointInOptimizedButton(int x, int y, const RECT& buttonRect);
    void updateOptimizedButtonHover(int x, int y, GlobalState& state);
    
    // 候選字視窗計算
    int calculateOptimalWindowWidth(const GlobalState& state);
    int calculateCandidateWindowHeight(const GlobalState& state);
    
    // 統一視窗定位
    void positionMainWindow(GlobalState& state);
    void positionWindowsOptimized(GlobalState& state);
    
    // OptimizedUI特定功能
    void updateBufferWindowPosition(GlobalState& state);
    void handleOptimizedToolbarDrag(HWND hwnd, POINT currentPos, GlobalState& state);
    void handleOptimizedCandidateDrag(HWND hwnd, POINT currentPos, GlobalState& state);
    
    // UI模式切換
    void switchToOptimizedUI(GlobalState& state);
    // 注意：switchToClassicUI 已移除，程序现在只使用 OptimizedUI
    
    // 半透明功能
    void applyTransparency(GlobalState& state);
    
    // 公共消息處理函數（用於消除重複代碼）
    LRESULT handleKeyboardInput(HWND hwnd, WPARAM wp);
    LRESULT handleTrayMessage(HWND hwnd, LPARAM lp);
    LRESULT handleCommand(HWND hwnd, WPARAM wp);
    LRESULT handleDisplayChange(HWND hwnd);
    LRESULT handleWindowDestroy(HWND hwnd);
}

#endif // WINDOW_MANAGER_H