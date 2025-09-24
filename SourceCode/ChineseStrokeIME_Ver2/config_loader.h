// config_loader.h - 設定檔載入
#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include "ime_core.h"

namespace ConfigLoader {
    // 載入介面設定
    void loadInterfaceConfig(GlobalState& state);
    
    // 載入所有設定
    void loadAllConfigs(GlobalState& state);
    
    // 重新載入設定
    void refreshConfigs(GlobalState& state);
}

#endif // CONFIG_LOADER_H
