/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file OraclePluginHandler.h
 * @brief Service class for the Oracle plug-in
 * @version 1.0.0.0
 * @date 2023-03-06
 * @author kWX884906
 */
#ifndef _ORACLE_PLUGIN_HANDLER_H
#define _ORACLE_PLUGIN_HANDLER_H
#include <thread>
#include <memory>
#include "common/Types.h"
#include "common/JsonHelper.h"
#include "apps/appprotect/plugininterface/ApplicationProtectPlugin_types.h"
#include "apps/appprotect/plugininterface/ApplicationService.h"
#include "servicecenter/thriftservice/include/IThriftService.h"

class OraclePluginHandler {
public:
    static OraclePluginHandler& GetInstance()
    {
        return m_Instance;
    }
    virtual ~OraclePluginHandler()
    {
        m_bTExitCheckArchiveArea = true;
        if (m_thOracleCheckAchive) {
            m_thOracleCheckAchive->join();
            m_thOracleCheckAchive.reset();
        }
    }

    mp_void CreateCheckArchiveThread();
    mp_void OracleUpdateDBInfo(const Json::Value& requestParam);

private:
    OraclePluginHandler()
    {}
    mp_void CheckArchiveThread();
    mp_void OracleCheckArchiveArea();
    void OracleQueryDBInfo(std::vector<AppProtect::OracleDBInfo>& vecDBInfo);
    std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> GetApplicationServiceClient(
        const std::shared_ptr<thriftservice::IThriftClient>& pThriftClient);
    mp_int32 OracleUpdateInitDbInfoJson(const Json::Value& requestParam, AppProtect::OracleDBInfo& dbInfo);
private:
    static OraclePluginHandler m_Instance;
    bool m_bTExitCheckArchiveArea = false;
    std::unique_ptr<std::thread> m_thOracleCheckAchive; // oracle业务，检查闪回区
};

#endif