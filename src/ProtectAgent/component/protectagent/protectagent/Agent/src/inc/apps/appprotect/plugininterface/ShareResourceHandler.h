#ifndef _SHARE_RESOURCE_HANDLER_H
#define _SHARE_RESOURCE_HANDLER_H

#include <memory>
#include "ShareResource.h"
#include "servicecenter/messageservice/include/RpcPublishObserver.h"
#include "servicecenter/servicefactory/include/IService.h"
#include "common/Defines.h"
#include "message/curlclient/DmeRestClient.h"
#include "message/curlclient/PmRestClient.h"
#include "jsoncpp/include/json/value.h"

namespace AppProtect {
class ShareResourceHandler :
    public ShareResourceIf,
    public messageservice::RpcPublishObserver,
    public servicecenter::IService {
public:
    ShareResourceHandler();
    ~ShareResourceHandler() {}
    EXTER_ATTACK void CreateResource(
        ActionResult& _return, const Resource& resource, const std::string& mainJobId) override;
    EXTER_ATTACK void QueryResource(
        ResourceStatus& _return, const Resource& resource, const std::string& mainJobId) override;
    EXTER_ATTACK void UpdateResource(
        ActionResult& _return, const Resource& resource, const std::string& mainJobId) override;
    EXTER_ATTACK void DeleteResource(
        ActionResult& _return, const Resource& resource, const std::string& mainJobId) override;
    EXTER_ATTACK void LockResource(
        ActionResult& _return, const Resource& resource, const std::string& mainJobId) override;
    EXTER_ATTACK void UnLockResource(
        ActionResult& _return, const Resource& resource, const std::string& mainJobId) override;
    EXTER_ATTACK void LockJobResource(
        ActionResult& _return, const Resource& resource, const std::string& mainJobId) override;
    bool Initailize() override;
    bool Uninitailize() override;
protected:
    void Update(std::shared_ptr<messageservice::RpcPublishEvent> event) override;
    void ThrowAppProtectException(int32_t errCode, const std::string &message);
    void CommonSendRequestHandle(ActionResult& _return, const DmeRestClient::HttpReqParam &req);
    bool CheckResourceJsonValid(const Json::Value &resourceInfo);
    void HandleQueryResourceResponse(ResourceStatus& _return, const Json::Value &resourceInfo);
    void SendRequestToPMHandle(ActionResult& _return, HttpReqCommonParam& req);
private:
    mp_string GetNodeId();
    mp_int32 AssignScopeKey(ResourceScope::type scope, std::string &scopeKey);
    mp_int32 SwitchResourceToJson(const Resource& resource, std::string &output);
    void PrintLog(const std::string &funName, const Resource &resource);

    mp_string m_nodeId;
};
}
#endif