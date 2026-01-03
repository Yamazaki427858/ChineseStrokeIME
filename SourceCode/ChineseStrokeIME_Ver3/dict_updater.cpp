// dict_updater.cpp - 字典更新器实现（使用WinINet API）
#include "dict_updater.h"
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <cctype>

// 注意：wininet.lib 已在 Makefile 中通过 -lwininet 链接

namespace DictUpdater {

// 将UTF-8字符串转换为宽字符串
std::wstring utf8ToWstring(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, NULL, 0);
    if (size <= 0) return L"";
    std::vector<wchar_t> buffer(size);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, buffer.data(), size);
    return std::wstring(buffer.data());
}

// 将宽字符串转换为UTF-8字符串
std::string wstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size <= 0) return "";
    std::vector<char> buffer(size);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.data(), size, NULL, NULL);
    return std::string(buffer.data());
}

// 下载文件主函数
DownloadResult downloadFromGitHub(const char* downloadUrl, const char* savePath, int timeoutSeconds) {
    DownloadResult result;
    
    // 使用默认值
    std::string url = downloadUrl ? downloadUrl : GITHUB_RAW_URL;
    std::string path = savePath ? savePath : TEMP_DICT_FILE;
    
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    
    try {
        // 初始化WinINet
        hInternet = InternetOpenA(
            "ChineseStrokeIME",
            INTERNET_OPEN_TYPE_PRECONFIG,
            NULL,
            NULL,
            0
        );
        
        if (!hInternet) {
            result.status = DownloadStatus::NetworkError;
            result.message = L"無法初始化網路連線";
            return result;
        }
        
        // 设置超时
        DWORD timeout = timeoutSeconds * 1000;
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_SEND_TIMEOUT, &timeout, sizeof(timeout));
        
        // 解析URL
        URL_COMPONENTSA urlComp;
        ZeroMemory(&urlComp, sizeof(urlComp));
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.dwHostNameLength = 1;
        urlComp.dwUrlPathLength = 1;
        urlComp.dwSchemeLength = 1;
        
        char hostName[256] = {0};
        char urlPath[2048] = {0};
        char scheme[16] = {0};
        
        urlComp.lpszHostName = hostName;
        urlComp.lpszUrlPath = urlPath;
        urlComp.lpszScheme = scheme;
        urlComp.dwHostNameLength = sizeof(hostName);
        urlComp.dwUrlPathLength = sizeof(urlPath);
        urlComp.dwSchemeLength = sizeof(scheme);
        
        if (!InternetCrackUrlA(url.c_str(), url.length(), 0, &urlComp)) {
            result.status = DownloadStatus::NetworkError;
            result.message = L"無效的URL格式";
            goto cleanup;
        }
        
        // 连接到服务器
        INTERNET_PORT port = urlComp.nPort;
        if (port == 0) {
            port = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT;
        }
        
        hConnect = InternetConnectA(
            hInternet,
            hostName,
            port,
            NULL,
            NULL,
            (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_SERVICE_HTTP : INTERNET_SERVICE_HTTP,
            0,
            0
        );
        
        if (!hConnect) {
            result.status = DownloadStatus::NetworkError;
            result.message = L"無法連接到伺服器";
            goto cleanup;
        }
        
        // 创建HTTP请求
        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
        if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
            flags |= INTERNET_FLAG_SECURE;
        }
        
        hRequest = HttpOpenRequestA(
            hConnect,
            "GET",
            urlPath,
            "HTTP/1.1",
            NULL,
            NULL,
            flags,
            0
        );
        
        if (!hRequest) {
            result.status = DownloadStatus::NetworkError;
            result.message = L"無法建立HTTP請求";
            goto cleanup;
        }
        
        // 发送请求
        if (!HttpSendRequestA(hRequest, NULL, 0, NULL, 0)) {
            result.status = DownloadStatus::NetworkError;
            result.message = L"無法發送HTTP請求";
            goto cleanup;
        }
        
        // 获取HTTP状态码
        DWORD statusCode = 0;
        DWORD statusCodeSize = sizeof(statusCode);
        HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                      &statusCode, &statusCodeSize, NULL);
        result.httpCode = statusCode;
        
        if (statusCode != HTTP_STATUS_OK) {
            result.status = DownloadStatus::HttpError;
            std::wstringstream ss;
            ss << L"HTTP錯誤：" << statusCode;
            result.message = ss.str();
            goto cleanup;
        }
        
        // 打开文件准备写入
        std::ofstream outFile(path, std::ios::binary);
        if (!outFile.is_open()) {
            result.status = DownloadStatus::FileError;
            result.message = L"無法建立臨時文件";
            goto cleanup;
        }
        
        // 读取数据并写入文件
        const size_t BUFFER_SIZE = 8192;
        std::vector<char> buffer(BUFFER_SIZE);
        DWORD bytesRead = 0;
        size_t totalBytes = 0;
        
        while (InternetReadFile(hRequest, buffer.data(), BUFFER_SIZE, &bytesRead)) {
            if (bytesRead == 0) break;
            outFile.write(buffer.data(), bytesRead);
            totalBytes += bytesRead;
        }
        
        outFile.close();
        
        if (!outFile.good() && totalBytes == 0) {
            result.status = DownloadStatus::FileError;
            result.message = L"文件寫入失敗";
            DeleteFileA(path.c_str());  // 删除失败的文件
            goto cleanup;
        }
        
        result.fileSize = totalBytes;
        result.status = DownloadStatus::Success;
        result.message = L"下載成功";
        
    } catch (...) {
        result.status = DownloadStatus::NetworkError;
        result.message = L"下載過程中發生異常";
    }
    
cleanup:
    if (hRequest) InternetCloseHandle(hRequest);
    if (hConnect) InternetCloseHandle(hConnect);
    if (hInternet) InternetCloseHandle(hInternet);
    
    return result;
}


// 安全更新字典
DownloadResult updateDictionarySafely(const char* downloadUrl, const char* localFile) {
    DownloadResult result;
    
    std::string dictFile = localFile ? localFile : LOCAL_DICT_FILE;
    std::string tempFile = TEMP_DICT_FILE;
    
    // 下载到临时文件
    result = downloadFromGitHub(downloadUrl, tempFile.c_str());
    
    if (result.status != DownloadStatus::Success) {
        return result;
    }
    
    // 验证下载的文件
    if (!validateDictFile(tempFile.c_str())) {
        result.status = DownloadStatus::FileError;
        result.message = L"下載的文件格式無效";
        DeleteFileA(tempFile.c_str());
        return result;
    }
    
    // 备份原文件
    std::string backupFile = dictFile + ".bak";
    CopyFileA(dictFile.c_str(), backupFile.c_str(), FALSE);
    
    // 替换原文件
    if (!MoveFileExA(tempFile.c_str(), dictFile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        // 如果替换失败，尝试恢复备份
        result.status = DownloadStatus::FileError;
        result.message = L"無法替換原文件";
        CopyFileA(backupFile.c_str(), dictFile.c_str(), FALSE);
        DeleteFileA(tempFile.c_str());
        return result;
    }
    
    // 删除备份文件
    DeleteFileA(backupFile.c_str());
    
    // 注意：不再保存 SHA（避免使用 GitHub API，防止访问次数限制）
    
    result.message = L"字典更新成功";
    return result;
}

// 验证字典文件格式
bool validateDictFile(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }
    
    std::string line;
    int validLines = 0;
    int maxCheckLines = 100;  // 检查前100行
    
    while (std::getline(file, line) && validLines < maxCheckLines) {
        // 跳过空行和注释
        if (line.empty() || line[0] == '#') continue;
        
        // 检查是否包含制表符分隔符
        size_t tabPos = line.find('\t');
        if (tabPos != std::string::npos && tabPos > 0 && tabPos < line.length() - 1) {
            validLines++;
        }
    }
    
    file.close();
    
    // 如果至少有一行有效数据，认为文件有效
    return validLines > 0;
}

// 获取状态消息
std::wstring getStatusMessage(const DownloadResult& result) {
    std::wstringstream ss;
    
    switch (result.status) {
        case DownloadStatus::Success:
            ss << L"下載成功：";
            ss << result.fileSize << L" 字節";
            break;
        case DownloadStatus::NetworkError:
            ss << L"網路錯誤：" << result.message;
            break;
        case DownloadStatus::FileError:
            ss << L"文件錯誤：" << result.message;
            break;
        case DownloadStatus::HttpError:
            ss << result.message;
            break;
        case DownloadStatus::Cancelled:
            ss << L"下載已取消";
            break;
        case DownloadStatus::Timeout:
            ss << L"下載超時";
            break;
        default:
            ss << L"未知錯誤：" << result.message;
    }
    
    return ss.str();
}

// 保存版本检查缓存
void saveVersionCache(const std::string& version, time_t checkTime) {
    std::ofstream outFile(VERSION_CACHE_FILE);
    if (!outFile.is_open()) return;
    
    outFile << version << "|" << checkTime;
    outFile.close();
}

// 加载版本检查缓存
std::string loadVersionCache(time_t& checkTime, int cacheHours) {
    std::ifstream inFile(VERSION_CACHE_FILE);
    if (!inFile.is_open()) return "";
    
    std::string line;
    std::getline(inFile, line);
    inFile.close();
    
    if (line.empty()) return "";
    
    // 解析格式：版本号|时间戳
    size_t pos = line.find('|');
    if (pos == std::string::npos || pos >= line.length() - 1) return "";
    
    std::string version = line.substr(0, pos);
    std::string timeStr = line.substr(pos + 1);
    
    try {
        checkTime = static_cast<time_t>(std::stoll(timeStr));
    } catch (...) {
        return "";
    }
    
    // 检查缓存是否过期
    time_t now = time(nullptr);
    time_t cacheAge = now - checkTime;
    int cacheSeconds = cacheHours * 3600;
    
    if (cacheAge < 0 || cacheAge > cacheSeconds) {
        // 缓存过期
        return "";
    }
    
    return version;
}

// 获取远程版本号（从 Update.md，带缓存机制）
std::string getRemoteVersion(const char* updateUrl, bool forceCheck, int cacheHours) {
    // 如果不强制检查，先尝试从缓存加载
    if (!forceCheck) {
        time_t cachedTime = 0;
        std::string cachedVersion = loadVersionCache(cachedTime, cacheHours);
        if (!cachedVersion.empty()) {
            return cachedVersion;  // 使用缓存
        }
    }
    
    // 需要从网络获取
    std::string url = updateUrl ? updateUrl : GITHUB_UPDATE_MD_URL;
    std::string version;
    
    HINTERNET hInternet = NULL;
    HINTERNET hConnect = NULL;
    HINTERNET hRequest = NULL;
    
    try {
        hInternet = InternetOpenA("ChineseStrokeIME", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
        if (!hInternet) return "";
        
        DWORD timeout = 10 * 1000; // 10秒超时
        InternetSetOptionA(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
        InternetSetOptionA(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));
        
        URL_COMPONENTSA urlComp;
        ZeroMemory(&urlComp, sizeof(urlComp));
        urlComp.dwStructSize = sizeof(urlComp);
        urlComp.dwHostNameLength = 1;
        urlComp.dwUrlPathLength = 1;
        urlComp.dwSchemeLength = 1;
        
        char hostName[256] = {0}, urlPath[2048] = {0}, scheme[16] = {0};
        urlComp.lpszHostName = hostName;
        urlComp.lpszUrlPath = urlPath;
        urlComp.lpszScheme = scheme;
        urlComp.dwHostNameLength = sizeof(hostName);
        urlComp.dwUrlPathLength = sizeof(urlPath);
        urlComp.dwSchemeLength = sizeof(scheme);
        
        if (!InternetCrackUrlA(url.c_str(), url.length(), 0, &urlComp)) goto cleanup;
        
        INTERNET_PORT port = urlComp.nPort ? urlComp.nPort : 
            ((urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? INTERNET_DEFAULT_HTTPS_PORT : INTERNET_DEFAULT_HTTP_PORT);
        
        hConnect = InternetConnectA(hInternet, hostName, port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
        if (!hConnect) goto cleanup;
        
        DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
        if (urlComp.nScheme == INTERNET_SCHEME_HTTPS) {
            flags |= INTERNET_FLAG_SECURE;
        }
        
        hRequest = HttpOpenRequestA(hConnect, "GET", urlPath, "HTTP/1.1", NULL, NULL, flags, 0);
        if (!hRequest) goto cleanup;
        
        if (!HttpSendRequestA(hRequest, NULL, 0, NULL, 0)) goto cleanup;
        
        DWORD statusCode = 0, statusCodeSize = sizeof(statusCode);
        HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusCodeSize, NULL);
        if (statusCode != HTTP_STATUS_OK) goto cleanup;
        
        // 读取响应内容
        const size_t BUFFER_SIZE = 4096;
        std::vector<char> buffer(BUFFER_SIZE);
        DWORD bytesRead = 0;
        std::string content;
        
        while (InternetReadFile(hRequest, buffer.data(), BUFFER_SIZE, &bytesRead)) {
            if (bytesRead == 0) break;
            content.append(buffer.data(), bytesRead);
        }
        
        // 解析版本号：查找 VERSION = XXX（大小写不敏感）
        std::string contentLower = content;
        std::transform(contentLower.begin(), contentLower.end(), contentLower.begin(), ::tolower);
        
        size_t versionPos = contentLower.find("version");
        if (versionPos != std::string::npos) {
            // 在原始内容中找到对应位置
            size_t equalsPos = content.find("=", versionPos);
            if (equalsPos == std::string::npos) {
                // 如果没找到 =，尝试查找下一个等号
                equalsPos = content.find("=", versionPos + 6); // "VERSION" 长度是 7
            }
            
            if (equalsPos != std::string::npos && equalsPos < content.length() - 1) {
                // 跳过等号和空格/制表符
                size_t start = equalsPos + 1;
                while (start < content.length() && (content[start] == ' ' || content[start] == '\t')) {
                    start++;
                }
                
                // 提取版本号（直到换行、回车或空格）
                size_t end = start;
                while (end < content.length() && 
                       content[end] != '\n' && content[end] != '\r' && 
                       content[end] != ' ' && content[end] != '\t') {
                    end++;
                }
                
                if (end > start) {
                    version = content.substr(start, end - start);
                    // 移除可能的尾随空格和制表符
                    while (!version.empty() && (version.back() == ' ' || version.back() == '\t' || 
                           version.back() == '\n' || version.back() == '\r')) {
                        version.pop_back();
                    }
                }
            }
        }
        
        // 如果成功获取版本号，保存到缓存
        if (!version.empty()) {
            saveVersionCache(version, time(nullptr));
        }
    } catch (...) {}
    
cleanup:
    if (hRequest) InternetCloseHandle(hRequest);
    if (hConnect) InternetCloseHandle(hConnect);
    if (hInternet) InternetCloseHandle(hInternet);
    return version;
}

} // namespace DictUpdater

