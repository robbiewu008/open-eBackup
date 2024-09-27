#ifndef AUTO_RELEASE_PLUGIN_H_
#define AUTO_RELEASE_PLUGIN_H_
#include "pluginfx/ExternalPluginManager.h"

class AutoReleasePlugin {
public:
    explicit AutoReleasePlugin(const mp_string& strAppType)
        : m_strAppType(strAppType)
    {}
    
    ~AutoReleasePlugin()
    {
        if (!m_strAppType.empty()) {
            ExternalPluginManager::GetInstance().ReleasePluginByRest(m_strAppType);
        }
    }
private:
    mp_string m_strAppType;
};
#endif