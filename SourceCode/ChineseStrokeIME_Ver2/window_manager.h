// window_manager.h - 視窗管理與繪製 (OptimizedUI支援版)
#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "ime_core.h"

namespace WindowManager {
    // 視窗註冊與建立
    bool registerWindowClasses(HINSTANCE hInstance);
    bool createWindows(HINSTANCE hInstance, GlobalState& state);
    
    // OptimizedUI視窗註冊與建立
    bool registerOptimizedWindowClasses(HINSTANCE hInstance);
    bool createOptimizedWindows(HINSTANCE hInstance, GlobalState& state);
    
    // 視窗程序
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT CALLBACK CandProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT CALLBACK BufferProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    
    // OptimizedUI視窗程序
    LRESULT CALLBACK OptimizedWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    LRESULT CALLBACK OptimizedCandProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    
	// 字碼視窗相關函數
    void drawInputWindow(HDC hdc, RECT rc, const GlobalState& state);
    void positionInputWindow(GlobalState& state);
    LRESULT CALLBACK InputProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
    bool createInputWindow(HINSTANCE hInstance, GlobalState& state);
	
    // 繪製函數
    void drawMain(HWND hwnd, HDC hdc, GlobalState& state);
    void drawCandidate(HWND hwnd, HDC hdc, const GlobalState& state);
    void drawBufferWindow(HDC hdc, RECT rc, GlobalState& state);
    void drawWithDoubleBuffering(HWND hwnd, HDC hdc, RECT rect,
                              void (*drawFunc)(HWND, HDC, GlobalState&),
                              GlobalState& state);
    
    // OptimizedUI繪製函數
    void drawOptimizedToolbar(HDC hdc, RECT rc, GlobalState& state);
    void drawOptimizedCandidate(HWND hwnd, HDC hdc, const GlobalState& state);
    
    // 按鈕繪製 (原有)
    void drawCloseButton(HDC hdc, RECT windowRect, GlobalState& state);
    void drawModeButton(HDC hdc, RECT windowRect, GlobalState& state);
    void drawCreditsButton(HDC hdc, RECT windowRect, GlobalState& state);
    void drawRefreshButton(HDC hdc, RECT windowRect, GlobalState& state);
    void drawBufferButton(HDC hdc, RECT windowRect, GlobalState& state);
    
    // 按鈕點擊檢測 (原有)
    bool isPointInCloseButton(int x, int y, const GlobalState& state);
    bool isPointInModeButton(int x, int y, const GlobalState& state);
    bool isPointInCreditsButton(int x, int y, const GlobalState& state);
    bool isPointInRefreshButton(int x, int y, const GlobalState& state);
    bool isPointInBufferButton(int x, int y, const GlobalState& state);
    bool isPointInSendButton(int x, int y, const GlobalState& state);
    bool isPointInClearButton(int x, int y, const GlobalState& state);
    bool isPointInSaveButton(int x, int y, const GlobalState& state);
	bool isPointInPrevPageButton(int x, int y, const GlobalState& state);
	bool isPointInNextPageButton(int x, int y, const GlobalState& state);
    
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
    void switchToClassicUI(GlobalState& state);
}

#endif // WINDOW_MANAGER_H