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
package openbackup.data.access.client.sdk.api.framework.agent;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentIqnValidateRequest;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentWwpnInfo;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CleanAgentLogReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CollectAgentLogRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.DeliverTaskStatusDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.GetAgentLogCollectStatusRsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.GetClusterEsnReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Rsp;
import openbackup.data.access.client.sdk.api.framework.agent.dto.PluginsDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentLevelReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentPluginTypeReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.model.AgentUpdatePluginTypeResult;
import openbackup.system.base.common.model.host.ManagementIp;
import openbackup.system.base.sdk.agent.model.AgentUpdateResponse;
import openbackup.system.base.sdk.cert.request.PushUpdateCertToAgentReq;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Param;
import feign.RequestLine;
import feign.Response;

import org.springframework.web.bind.annotation.RequestBody;

import java.net.URI;
import java.util.Map;

/**
 * Agent unified Rest Api
 *
 * @author 30009433
 * @version [OceanProtect A8000 1.2.0]
 * @since 2022-05-19
 */
public interface AgentUnifiedRestApi {
    /**
     * 向Agent查询应用详细信息
     *
     * @param uri agent接口访问地址
     * @param appType 应用类型
     * @param listResourceV2Req 查询app list的参数
     * @return agent主机信息
     */
    @ExterAttack
    @RequestLine("POST /v2/agent/{appType}/detail")
    ListResourceV2Rsp listResourceDetailV2(URI uri, @Param("appType") String appType,
        @RequestBody ListResourceV2Req listResourceV2Req);

    /**
     * 获取agent主机信息
     *
     * @param uri agent接口访问地址
     * @return agent主机信息
     */
    @ExterAttack
    @RequestLine("GET /v1/agent/host")
    HostDto getHost(URI uri);

    /**
     * 获取agent主机信息
     * 这个接口兼容性最好
     *
     * @param uri agent接口访问地址
     * @return agent主机信息
     */
    @ExterAttack
    @RequestLine("GET /agent/host")
    HostDto getAgentHost(URI uri);

    /**
     * 获取agent插件信息
     *
     * @param uri agent接口访问地址
     * @return agent插件信息
     */
    @ExterAttack
    @RequestLine("GET /v1/agent/host/applugins")
    PluginsDto getPlugins(URI uri);

    /**
     * 修改 agent主机集群信息
     *
     * @param uri agent接口访问地址
     * @param managementIp agent主机集群信息
     */
    @RequestLine("PUT /v1/agent/host/managerserver")
    void modifyAgentServerList(URI uri, ManagementIp managementIp);

    /**
     * 向Agent查询应用详细信息
     *
     * @param uri agent接口访问地址
     * @param appType 应用类型
     * @param listResourceReq 查询app list的参数
     * @return agent主机信息
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/{appType}/detail")
    AgentDetailDto getDetail(URI uri, @Param("appType") String appType, @RequestBody ListResourceReq listResourceReq);

    /**
     * check application status, connection information
     *
     * @param uri agent接口访问地址
     * @param appType 应用类型
     * @param checkAppReq check app信息
     * @return agent主机信息
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/{appType}/check")
    AgentBaseDto check(URI uri, @Param("appType") String appType, @RequestBody CheckAppReq checkAppReq);

    /**
     * Query the cluster information of agent
     *
     * @param uri agent接口访问地址
     * @param appType 应用类型
     * @param checkAppReq check app 信息
     * @return cluster information 集群信息
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/{appType}/cluster")
    AppEnvResponse queryCluster(URI uri, @Param("appType") String appType, @RequestBody CheckAppReq checkAppReq);

    /**
     * 向agent查询应用配置信息
     *
     * @param uri agent接口访问地址
     * @param appType 应用类型
     * @param script 脚本
     * @return 配置
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/{appType}/config?script={script}")
    Map<String, Object> queryAppConfig(URI uri, @Param("appType") String appType, @Param("script") String script);

    /**
     * 手动传递任务的状态到agent
     *
     * @param uri agent接口访问地址
     * @param appType 应用类型
     * @param deliverTaskStatusDto 请求体
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/{appType}/task-status")
    void deliverTaskStatus(URI uri, @Param("appType") String appType,
        @RequestBody DeliverTaskStatusDto deliverTaskStatusDto);

    /**
     * 收集Agent日志
     *
     * @param uri agent接口访问地址
     * @return 收集记录信息
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/log/collect")
    CollectAgentLogRsp collectAgentLog(URI uri);

    /**
     * 查询Agent收集状态
     *
     * @param uri agent接口访问地址
     * @return 状态
     */
    @ExterAttack
    @RequestLine("GET /v1/agent/host/log/collecting-status")
    GetAgentLogCollectStatusRsp getCollectAgentLogStatus(URI uri);

    /**
     * 导出Agent日志
     *
     * @param uri agent接口访问地址
     * @param id 记录ID
     * @param maxSize 日志大小规格
     * @return 日志流
     */
    @ExterAttack
    @RequestLine("GET /v1/agent/host/log/export?id={id}&maxSize={maxSize}")
    Response exportAgentLog(URI uri, @Param("id") String id, @Param("maxSize") long maxSize);

    /**
     * 更新Agent日志级别
     *
     * @param uri agent接口访问地址
     * @param configReq 配置更新请求
     */
    @ExterAttack
    @RequestLine("PUT /v1/agent/host/log/level")
    void updateAgentLogLevel(URI uri, @RequestBody UpdateAgentLevelReq configReq);

    /**
     * 通知Agent清理日志
     *
     * @param uri agent接口访问地址
     * @param cleanAgentLogReq 请求
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/log/clean")
    void cleanAgentLog(URI uri, @RequestBody CleanAgentLogReq cleanAgentLogReq);

    /**
     * 查询agent wwpn
     *
     * @param uri agent接口访问地址
     * @return AgentWwpnInfo agentWwpnInfo
     */
    @ExterAttack
    @RequestLine("GET /v1/agent/host/wwpns")
    AgentWwpnInfo listWwpn(URI uri);

    /**
     * 查询agent wwpn
     *
     * @param uri agent接口访问地址
     * @return AgentWwpnInfo agentWwpnInfo
     */
    @ExterAttack
    @RequestLine("GET /v2/agent/host/wwpns")
    AgentWwpnInfo listWwpnV2(URI uri);

    /**
     * 获取Iqn状态列表
     *
     * @param uri uri
     * @param agentIqnValidateRequest agentIqnValidateRequest
     * @return AgentWwpnInfo AgentWwpnInfo
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/iqns")
    AgentWwpnInfo getIqnInfoList(URI uri, @RequestBody AgentIqnValidateRequest agentIqnValidateRequest);

    /**
     * 扫描SanClient Iqn
     *
     * @param uri uri
     * @return String SanClient Iqn
     */
    @ExterAttack
    @RequestLine("GET /v1/agent/host/iqns/scan")
    AgentWwpnInfo scanSanClientIqnInfo(URI uri);

    /**
     * agent扫盘
     *
     * @param uri agent接口访问地址
     * @return AgentBaseDto agentBaseDto
     */
    @ExterAttack
    @RequestLine("PUT /v1/agent/host/dataturbo/rescan")
    AgentBaseDto reScan(URI uri);

    /**
     * 修改主机支持资源类型
     *
     * @param uri agent接口访问地址
     * @param updateAgentPluginTypeReq 修改资源类型请求体
     * @return AgentBaseDto agentBaseDto
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/action/modify-plugin")
    AgentUpdateResponse modifyPlugin(URI uri, @RequestBody UpdateAgentPluginTypeReq updateAgentPluginTypeReq);

    /**
     * 查询修改主机资源类型结果
     *
     * @param uri agent接口访问地址
     * @return result
     */
    @ExterAttack
    @RequestLine("GET /agent/host/action/check/status/modify")
    AgentUpdatePluginTypeResult getModifyPluginTypeResult(URI uri);

    /**
     * 获取绑定关系集群esn
     *
     * @param uri agent接口访问地址
     * @return GetClusterEsnReq getClusterEsnReq
     */
    @ExterAttack
    @RequestLine("GET /v1/agent/tasks/get-esn")
    GetClusterEsnReq getClusterEsn(URI uri);

    /**
     * agent解挂载
     *
     * @param uri agent接口访问地址
     * @param appType 应用类型
     * @param reqBody 请求体
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/{appType}/remove-protect")
    void removeProtectUnmountRepo(URI uri, @Param("appType") String appType, @RequestBody String reqBody);

    /**
     * 给agent推送证书
     *
     * @param uri agent接口访问地址
     * @param pushUpdateCertToAgentReq 请求体
     * @return AgentBaseDto
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/action/cert/push")
    AgentBaseDto pushCertToAgent(URI uri, @RequestBody PushUpdateCertToAgentReq pushUpdateCertToAgentReq);

    /**
     * 通知agent更新证书
     *
     * @param uri agent接口访问地址
     * @param pushUpdateCertToAgentReq 请求体
     * @return AgentBaseDto
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/action/cert/update")
    AgentBaseDto notifyAgentUpdateCert(URI uri, @RequestBody PushUpdateCertToAgentReq pushUpdateCertToAgentReq);

    /**
     * 通知agent回退证书
     *
     * @param uri agent接口访问地址
     * @param pushUpdateCertToAgentReq 请求体
     * @return AgentBaseDto
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/action/cert/fallback")
    AgentBaseDto notifyAgentFallbackCert(URI uri, @RequestBody PushUpdateCertToAgentReq pushUpdateCertToAgentReq);

    /**
     * 通知agent删除旧证书
     *
     * @param uri agent接口访问地址
     * @param pushUpdateCertToAgentReq 请求体
     * @return AgentBaseDto
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/action/cert/delete-old-cert")
    AgentBaseDto notifyAgentDeleteOldCert(URI uri, @RequestBody PushUpdateCertToAgentReq pushUpdateCertToAgentReq);

    /**
     * pm检查与agent的连通性
     *
     * @param uri agent接口访问地址
     * @return AgentBaseDto
     */
    @ExterAttack
    @RequestLine("POST /v1/agent/host/action/cert/network/check")
    AgentBaseDto checkConnection(URI uri);
}