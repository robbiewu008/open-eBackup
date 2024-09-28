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
package openbackup.data.access.framework.core.agent;

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
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentLevelReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.UpdateAgentPluginTypeReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.model.AgentUpdatePluginTypeResult;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.annotation.Routing;
import openbackup.system.base.common.model.host.ManagementIp;
import openbackup.system.base.sdk.agent.model.AgentUpdateResponse;
import openbackup.system.base.sdk.cert.request.PushUpdateCertToAgentReq;

import feign.Response;

import java.util.List;
import java.util.Map;

/**
 * Agent Unified Service
 *
 */
public interface AgentUnifiedService {
    /**
     * Agent分页查询 详细信息
     *
     * @param appType 应用类型
     * @param endpoint 请求ip
     * @param port 请求port
     * @param listResourceV2Req 查询app list的参数
     * @return 按条件查询出的详细信息 分页对象
     */
    @Routing(destinationIp = "#{endpoint}")
    default PageListResponse<ProtectedResource> getDetailPageList(String appType, String endpoint, Integer port,
        ListResourceV2Req listResourceV2Req) {
        return getDetailPageList(appType, endpoint, port, listResourceV2Req, false);
    }

    /**
     * Agent分页查询 详细信息
     *
     * @param appType 应用类型
     * @param endpoint 请求ip
     * @param port 请求port
     * @param listResourceV2Req 查询app list的参数
     * @param isUseLongTimeApi 是否使用超时时间较长的agentApi
     * @return 按条件查询出的详细信息 分页对象
     */
    PageListResponse<ProtectedResource> getDetailPageList(String appType, String endpoint, Integer port,
        ListResourceV2Req listResourceV2Req, boolean isUseLongTimeApi);

    /**
     * Agent分页查询 详细信息
     *
     * @param appType 应用类型
     * @param endpoint 请求ip
     * @param port 请求port
     * @param listResourceV2Req 查询app list的参数
     * @param isUseLongTimeApi 是否使用超时时间较长的agentApi
     * @return 按条件查询出的详细信息 分页对象
     */
    PageListResponse<ProtectedResource> getDetailPageListNoRetry(String appType, String endpoint, Integer port,
        ListResourceV2Req listResourceV2Req, boolean isUseLongTimeApi);

    /**
     * Agent查询 详细信息
     *
     * @param appType 应用类型
     * @param endpoint 请求ip
     * @param port 请求port
     * @param listResourceReq 查询app list的参数
     * @return 查询出的详细信息
     */
    AgentDetailDto getDetail(String appType, String endpoint, Integer port, ListResourceReq listResourceReq);

    /**
     * Agent查询 详细信息
     *
     * @param appType 应用类型
     * @param endpoint 请求ip
     * @param port 请求port
     * @param listResourceReq 查询app list的参数
     * @return 查询出的详细信息
     */
    AgentDetailDto getDetailNoRetry(String appType, String endpoint, Integer port, ListResourceReq listResourceReq);

    /**
     * 获取Agent主机信息
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return agent返回的主机信息
     */
    HostDto getHost(String endpoint, Integer port);

    /**
     * 获取Agent主机信息
     * 这个接口兼容性最好
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return agent返回的主机信息
     */
    HostDto getAgentHost(String endpoint, Integer port);

    /**
     * 获取Agent插件信息
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return agent返回的插件信息
     */
    List<ProtectedResource> getPlugins(String endpoint, Integer port);

    /**
     * 检查应用
     *
     * @param protectedResource 检查的资源或环境
     * @param agent agent代理
     * @return 响应
     */
    AgentBaseDto checkApplication(ProtectedResource protectedResource, ProtectedEnvironment agent);

    /**
     * 检查应用
     *
     * @param protectedResource 检查的资源或环境
     * @param agent agent代理
     * @return 响应
     */
    AgentBaseDto checkApplicationNoRetry(ProtectedResource protectedResource, ProtectedEnvironment agent);

    /**
     * 检查应用
     *
     * @param protectedResource 检查的资源或环境
     * @param agent agent代理
     * @param checkAppReq checkAppReq
     * @return 响应
     */
    AgentBaseDto check(ProtectedResource protectedResource, ProtectedEnvironment agent, CheckAppReq checkAppReq);

    /**
     * 根据资源类型检查
     *
     * @param subType 检查资源子类型
     * @param agent agent代理
     * @param checkAppReq checkAppReq
     * @return 响应
     */
    AgentBaseDto check(String subType, ProtectedEnvironment agent, CheckAppReq checkAppReq);

    /**
     * 根据资源类型检查
     *
     * @param subType 检查资源子类型
     * @param endpoint 地址
     * @param port 端口
     * @param checkAppReq 检查req
     * @return 响应
     */
    AgentBaseDto check(String subType, String endpoint, Integer port, CheckAppReq checkAppReq);

    /**
     * 检查应用
     *
     * @param protectedResource 检查的资源或环境
     * @param agent agent代理
     * @param checkAppReq checkAppReq
     * @return 响应
     */
    AgentBaseDto checkNoRetry(ProtectedResource protectedResource, ProtectedEnvironment agent, CheckAppReq checkAppReq);

    /**
     * 查询集群信息
     *
     * @param protectedResource 检查的资源
     * @param protectedEnvironment 检查的agent环境
     * @return 集群信息
     */
    AppEnvResponse getClusterInfo(ProtectedResource protectedResource, ProtectedEnvironment protectedEnvironment);

    /**
     * 查询集群信息
     *
     * @param subType 子资源类型
     * @param environment agent代理
     * @param checkAppReq checkAppReq
     * @param isRetry 是否重试
     * @return 集群信息
     */
    AppEnvResponse getClusterInfo(String subType, ProtectedEnvironment environment, CheckAppReq checkAppReq,
        boolean isRetry);

    /**
     * 查询集群信息
     *
     * @param protectedResource 检查的资源
     * @param protectedEnvironment 检查的agent环境
     * @return 集群信息
     */
    AppEnvResponse getClusterInfoNoRetry(ProtectedResource protectedResource,
        ProtectedEnvironment protectedEnvironment);

    /**
     * 查询应用配置信息
     *
     * @param appType 应用类型
     * @param script 脚本
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return 配置信息
     */
    Map<String, Object> queryAppConf(String appType, String script, String endpoint, Integer port);

    /**
     * 传递任务信息
     *
     * @param appType 应用类型
     * @param deliverTaskStatusDto 请求体
     * @param endpoint agent的Ip
     * @param port agent的port
     */
    void deliverTaskStatus(String appType, DeliverTaskStatusDto deliverTaskStatusDto, String endpoint, Integer port);

    /**
     * 收集Agent日志
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return 收集记录信息
     */
    CollectAgentLogRsp collectAgentLog(String endpoint, Integer port);

    /**
     * 查询Agent收集状态
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return 状态
     */
    GetAgentLogCollectStatusRsp getCollectAgentLogStatus(String endpoint, Integer port);

    /**
     * 导出Agent日志
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @param id 记录ID
     * @param maxSize 日志大小规格
     * @return 日志流
     */
    Response exportAgentLog(String endpoint, Integer port, String id, long maxSize);

    /**
     * 更新Agent日志级别
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @param configReq 配置更新请求
     */
    void updateAgentLogLevel(String endpoint, Integer port, UpdateAgentLevelReq configReq);

    /**
     * 查询agent WWPN
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @param version 接口版本
     * @return AgentWwpnInfo agentWwpnInfo
     */
    AgentWwpnInfo listWwpn(String endpoint, Integer port, String version);

    /**
     * 获取iqn信息
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @param agentIqnValidateRequest agentIqnValidateRequest
     * @return AgentWwpnInfo agentWwpnInfo
     */
    AgentWwpnInfo getIqnInfoList(String endpoint, Integer port, AgentIqnValidateRequest agentIqnValidateRequest);

    /**
     * 获取SanClient IQN信息 一个获取SanClient主机最多只有一个IQN
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return AgentWwpnInfo SanClient主机的IQN
     */
    AgentWwpnInfo scanSanClientIqnInfo(String endpoint, Integer port);

    /**
     * agent扫盘
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return AgentBaseDto base
     */
    AgentBaseDto reScan(String endpoint, Integer port);

    /**
     * 清理agent日志
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @param cleanAgentLogReq 请求
     */
    void cleanAgentLog(String endpoint, Integer port, CleanAgentLogReq cleanAgentLogReq);

    /**
     * 创建agent修改插件任务
     *
     * @param endpoint ip
     * @param port port
     * @param req agent信息
     * @return 修改应用类型返回结果
     */
    AgentUpdateResponse modifyPluginType(String endpoint, Integer port, UpdateAgentPluginTypeReq req);

    /**
     * 查询agent修改插件结果
     *
     * @param endpoint ip
     * @param port port
     * @return 查看应用类型返回结果
     */
    AgentUpdatePluginTypeResult getModifyPluginTypeResult(String endpoint, Integer port);

    /**
     * 通过agent 查出绑定的集群esn
     *
     * @param endpoint agent的Ip
     * @param port agent的port
     * @return 集群esn
     */
    String queryRelatedClusterEsnByAgent(String endpoint, Integer port);

    /**
     * Remove protect unmount repo
     *
     * @param endpoint endpoint
     * @param port port
     * @param appType appType
     * @param reqBody reqBody
     */
    void removeProtectUnmountRepo(String endpoint, Integer port, String appType, String reqBody);

    /**
     * Remove protect unmount repo
     *
     * @param endpoint endpoint
     * @param port port
     * @param appType appType
     * @param reqBody reqBody
     */
    void removeProtectUnmountRepoNoRetry(String endpoint, Integer port, String appType, String reqBody);

    /**
     * 推送证书给agent
     *
     * @param agent agent环境信息
     * @param pushUpdateCertToAgentReq 封装证书的参数
     * @return AgentBaseDto
     */
    AgentBaseDto pushCertToAgent(ProtectedEnvironment agent, PushUpdateCertToAgentReq pushUpdateCertToAgentReq);

    /**
     * 通知agent更新证书
     *
     * @param agent agent环境信息
     * @param pushUpdateCertToAgentReq 封装证书的参数
     * @return AgentBaseDto
     */
    AgentBaseDto notifyAgentUpdateCert(ProtectedEnvironment agent, PushUpdateCertToAgentReq pushUpdateCertToAgentReq);

    /**
     * 通知agent回退证书
     *
     * @param agent agent环境信息
     * @param pushUpdateCertToAgentReq 封装证书的参数
     * @return AgentBaseDto
     */
    AgentBaseDto notifyAgentFallbackCert(ProtectedEnvironment agent, PushUpdateCertToAgentReq pushUpdateCertToAgentReq);

    /**
     * 通知agent删除旧证书
     *
     * @param agent agent环境信息
     * @param pushUpdateCertToAgentReq 封装证书的参数
     * @return AgentBaseDto
     */
    AgentBaseDto notifyAgentDeleteOldCert(ProtectedEnvironment agent,
        PushUpdateCertToAgentReq pushUpdateCertToAgentReq);

    /**
     * pm检查与agent的连通性
     *
     * @param agent agent环境信息
     * @return AgentBaseDto
     */
    AgentBaseDto checkConnection(ProtectedEnvironment agent);

    /**
     * 更新AgentServerIp
     *
     * @param agent agent
     * @param managementIp serverIp列表
     */
    void updateAgentServer(ProtectedEnvironment agent, ManagementIp managementIp);
}
