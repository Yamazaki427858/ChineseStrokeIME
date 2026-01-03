// ime_core.cpp - 核心工具函數實作
#include "ime_core.h"
#include <algorithm>

namespace Utils {
    std::wstring utf8ToWstr(const std::string& str) {
        if (str.empty()) return std::wstring();
        int sz = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
        if (sz <= 0) return std::wstring();
        std::wstring res(sz, 0);
        MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), &res[0], sz);
        return res;
    }

    std::string wstrToUtf8(const std::wstring& ws) {
        if (ws.empty()) return std::string();
        int sz = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
        if (sz <= 0) return std::string();
        std::string res(sz, 0);
        WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), &res[0], sz, nullptr, nullptr);
        return res;
    }

    void updateStatus(GlobalState& state, const std::wstring& msg) {
        state.statusInfo = msg;
        if (state.hWnd) InvalidateRect(state.hWnd, nullptr, TRUE);
    }

    bool isPunctuation(const std::wstring& word) {
        if (word.empty()) return false;
        
        std::wstring punctuations = L"，。？！：；（）「」『』《》〈〉【】：—……\"\"''｜＼－～＿￥％＃＄［］"
                                   L",.?!:;()[]{}\\'\"'<>/\\-_@#$%^&*+=|`~"
                                   L"　";
        
        for (wchar_t ch : word) {
            if (ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\r' || ch == L'　') {
                continue;
            }
            if (punctuations.find(ch) == std::wstring::npos) {
                return false;
            }
        }
        return true;
    }

    COLORREF parseColorFromString(const std::string& colorStr) {
        if (colorStr.empty()) return RGB(0,0,0);
        
        if (colorStr[0] == '#' && colorStr.length() == 7) {
            unsigned long rgb = strtoul(colorStr.substr(1).c_str(), nullptr, 16);
            return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
        }
        
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
}
namespace Utils {
    bool isPointInRect(int x, int y, const RECT& rect) {
        return (x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom);
    }
}

// 注意：全域變數實例已移除，只保留在main.cpp中定義
// 這裡不再重複定義 g_state 和 g_hKeyboardHook