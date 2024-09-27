/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef QUERY_CLUSTER_H
#define QUERY_CLUSTER_H
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "define/Types.h"
#include "common/Defines.h"
#include "common/JsonHelper.h"

namespace GeneralDB {
class ClusterOperation {
public:
    ClusterOperation() = default;
    ~ClusterOperation() = default;

    static void DiscoverHostCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv);
    static void DiscoverAppCluster(ApplicationEnvironment& returnEnv, const ApplicationEnvironment& appEnv,
        const Application& application);

    /**
     *  @brief 应用检查
     *  @param returnValue 执行返回的结果
     *  @param appEnv 应用环境
     *  @param application 应用类型
     * */
    static mp_void CheckApplication(ActionResult& returnValue,
        const ApplicationEnvironment& appEnv, const Application& application);
    
    /**
     *  @brief 列举资源，不分页
     *  @param returnValue 执行返回的结果
     *  @param appEnv 应用环境
     *  @param application 应用类型
     *  @param parentResource 父资源
     * */
    static mp_void ListApplicationResource(std::vector<ApplicationResource>& returnValue,
        const ApplicationEnvironment& appEnv, const Application& application,
        const ApplicationResource& parentResource);

    /**
     *  @brief 列举资源，分页
     *  @param returnValue 执行返回的结果
     *  @param ListResourceRequest 包含资源环境，已经环境类型，已经分页策略
     * */
    static mp_void ListApplicationResourceV2(AppProtect::ResourceResultByPage& returnValue,
        const AppProtect::ListResourceRequest& request);

    /**
     *  @brief oracle业务，查询归档空间使用率是否超过阈值
     *  @param returnValue 执行返回的结果
     *  @param appType 当前固定为Oracle
     * */
    static mp_void OracleCheckArchiveArea(ActionResult& _return,
    const std::string& appType, const std::vector<AppProtect::OracleDBInfo>& dbInfoList);

    static mp_void ListApplicationConfig(std::map<std::string, std::string>& returnValue,
        const std::string& script);
    /**
     *  @brief 移除保护时去挂载日志文件系统
     *  @param returnValue 执行返回的结果
     *  @param appEnv 应用环境
     *  @param application 应用类型
     * */
    static mp_void RemoveProtect(ActionResult& returnValue,
        const ApplicationEnvironment& appEnv, const Application& application);

private:
    static mp_void GetException(const Json::Value &retValue);
};
}
#endif