#ifndef AGENT_SERVICE_PLUGIN_EX_H
#define AGENT_SERVICE_PLUGIN_EX_H

#include <regex>
#include "common/Defines.h"
#include "plugins/ServicePlugin.h"

template<typename T, typename = typename std::enable_if<std::is_base_of<CServicePlugin, T>::value>::type>
class RestActionMapEx : public CRestActionMap<T> {
public:
    mp_int32 GetAction(
        mp_string& strUrl, const mp_string& strMethod, typename CRestActionMap<T>::rest_action_t& restAction) override
    {
        mp_string strKey = strUrl + "_" + strMethod;

        // return if string is same for reduce executing performce
        auto iter = CRestActionMap<T>::m_mapActions.find(strKey);
        if (iter != CRestActionMap<T>::m_mapActions.end()) {
            restAction = iter->second;
            return MP_SUCCESS;
        }

        // foreach for regex
        for (auto& item : CRestActionMap<T>::m_mapActions) {
            mp_string pattern = item.first;
            std::regex reg(pattern, std::regex::ECMAScript | std::regex::icase);
            std::smatch ret;
            if (std::regex_search(strKey, ret, reg)) {
                restAction = item.second;
                return MP_SUCCESS;
            }
        }

        ERRLOG("Can not find url:%s_%s", strUrl.c_str(), strMethod.c_str());
        return MP_FAILED;
    }
};

#define REGISTER_PLUGIN_EX(clsname)                                                                         \
    extern "C" AGENT_EXPORT IPlugin* QueryInterface()                                                       \
    {                                                                                                       \
        COMMLOG(OS_LOG_INFO, "Create new plugin obj %s.", #clsname);                                        \
        return new (std::nothrow) clsname();                                                                \
    }                                                                                                       \
    static RestActionMapEx<clsname> restActionMap;                                                          \
    static CDppActionMap<clsname> dppActionMap

#endif
