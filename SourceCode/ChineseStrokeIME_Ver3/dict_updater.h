// dict_updater.h - 字典更新器（支持从GitHub下载更新）
#ifndef DICT_UPDATER_H
#define DICT_UPDATER_H

#include "ime_core.h"
#include <string>

namespace DictUpdater {
    // 下载状态枚举
    enum class DownloadStatus {
        Success,           // 下载成功
        NetworkError,      // 网络错误
        FileError,         // 文件错误
        HttpError,         // HTTP错误
        Cancelled,         // 已取消
        Timeout            // 超时
    };
    
    // 下载结果结构
    struct DownloadResult {
        DownloadStatus status;
        std::wstring message;
        int httpCode;
        size_t fileSize;
        
        DownloadResult() : status(DownloadStatus::NetworkError), httpCode(0), fileSize(0) {}
    };
    
    // GitHub URLs 常量
    const char* const GITHUB_RAW_URL = 
        "https://raw.githubusercontent.com/Yamazaki427858/ChineseStrokeIME/ChineseStrokeIME/SourceCode/%E5%AD%97%E7%A2%BC/Zi-Ma-Biao.txt";
    const char* const GITHUB_UPDATE_MD_URL = 
        "https://raw.githubusercontent.com/Yamazaki427858/ChineseStrokeIME/refs/heads/ChineseStrokeIME/Update.md";
    const char* const GITHUB_REPO_URL = 
        "https://github.com/Yamazaki427858/ChineseStrokeIME";
    
    // 本地文件名
    const char* const LOCAL_DICT_FILE = "Zi-Ma-Biao.txt";
    const char* const TEMP_DICT_FILE = "Zi-Ma-Biao.txt.tmp";
    const char* const VERSION_CACHE_FILE = "version_cache.txt";  // 版本检查缓存文件
    
    // 从GitHub下载字码表
    // downloadUrl: 下载URL（如果为空则使用默认GitHub URL）
    // savePath: 保存路径（如果为空则使用默认文件名）
    // timeoutSeconds: 超时时间（秒），默认30秒
    DownloadResult downloadFromGitHub(const char* downloadUrl = nullptr, 
                                      const char* savePath = nullptr,
                                      int timeoutSeconds = 30);
    
    
    // 安全更新字典：下载到临时文件，验证后替换原文件
    // downloadUrl: 下载URL（如果为空则使用默认GitHub URL）
    // localFile: 本地文件路径（如果为空则使用默认文件名）
    DownloadResult updateDictionarySafely(const char* downloadUrl = nullptr,
                                         const char* localFile = nullptr);
    
    // 验证下载的文件是否有效（检查格式和内容）
    bool validateDictFile(const char* filePath);
    
    // 获取错误消息字符串
    std::wstring getStatusMessage(const DownloadResult& result);
    
    // 获取远程版本号（从 Update.md，带缓存机制）
    // updateUrl: Update.md 的 URL（如果为空则使用默认URL）
    // forceCheck: 是否强制检查（忽略缓存），默认 false
    // cacheHours: 缓存有效期（小时），默认 24 小时
    // 返回版本号字符串，失败返回空字符串
    std::string getRemoteVersion(const char* updateUrl = nullptr, bool forceCheck = false, int cacheHours = 24);
    
    // 保存版本检查缓存（内部使用）
    void saveVersionCache(const std::string& version, time_t checkTime);
    
    // 加载版本检查缓存（内部使用）
    // 返回缓存的版本号，如果缓存过期或不存在返回空字符串
    std::string loadVersionCache(time_t& checkTime, int cacheHours = 24);
}

#endif // DICT_UPDATER_H

