#ifndef APPLICATION_PROTECT_PLUGIN_H
#define APPLICATION_PROTECT_PLUGIN_H

#include <vector>
#include "plugins/ServicePlugin.h"
#include "apps/appprotect/AppProtectService.h"

class AppProtectPlugin : public CServicePlugin {
public:
    AppProtectPlugin();
    virtual ~AppProtectPlugin();
    virtual mp_int32 Initialize(std::vector<mp_uint32>& cmds);
    mp_int32 DoAction(CRequestMsg& req, CResponseMsg& rsp);

private:
    EXTER_ATTACK mp_int32 PluginResourceV1(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 PluginDetailV1(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 PluginCheckV1(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 PluginClusterV1(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 PluginDetailV2(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 WakeUpJob(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 SanclientJob(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 SanclientJobForUbc(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 SanclientCleanJob(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 AbortJob(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 PluginConfigV1(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 DeliverJobStatus(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 GetESN(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 RemoveProtect(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 InitializeExternalPlugMgr();
    mp_void DealInvokePluginFailed(mp_int32 iRet, CResponseMsg& rsp);

private:
    mp_int32 SanclientPreParamCheck(const Json::Value& jvReq);
    mp_int32 SanclientPreParamCheckIsVaild(const Json::Value& jvReq);
    mp_int32 SanclientPrepareJob(const Json::Value& jvReq);
private:
    thread_id_t m_CheckSanclientThread;
};

#endif
