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
package openbackup.system.base.sdk.infrastructure;

import openbackup.system.base.bean.ConfigMapOperationParam;
import openbackup.system.base.bean.FileSyncEntity;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.model.ha.AddStandbyHaRequest;
import openbackup.system.base.sdk.infrastructure.model.InfraAddServiceAndModifyComponentIpRequest;
import openbackup.system.base.sdk.infrastructure.model.InfraConfigMapRequest;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.InfrastructureResponse;
import openbackup.system.base.sdk.infrastructure.model.beans.CertificateTypeInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.ClusterHaStatusResponse;
import openbackup.system.base.sdk.infrastructure.model.beans.ClusterNetworkRequest;
import openbackup.system.base.sdk.infrastructure.model.beans.LogLevelInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NetworkCheckRequest;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeControllerInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.PostClusterHaRequest;
import openbackup.system.base.sdk.infrastructure.model.beans.SftpUserInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.SftpUsernameInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.SwitchRequest;
import openbackup.system.base.sdk.infrastructure.model.beans.UpdateHaConfigInfraRequest;
import openbackup.system.base.sdk.infrastructure.model.beans.VipInfo;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * Target cluster service
 *
 * @author t00508428
 * @since 2020-12-17
 */
@FeignClient(name = "InfrastructureRestApi", url = "${service.url.infra}",
    configuration = CommonFeignConfiguration.class)
public interface InfrastructureRestApi {
    /**
     * 基础设施备份能力
     *
     * @param subSystem 子系统名
     * @param dataType 数据类型名
     * @param path 备份路径
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/data/backup")
    @ResponseBody
    InfrastructureResponse backup(@RequestParam("subsystem") String subSystem,
        @RequestParam("data_type") String dataType, @RequestParam("path") String path);

    /**
     * 基础设施恢复能力
     *
     * @param subSystem 子系统名
     * @param dataType 数据类型名
     * @param path 恢复路径
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/data/recover")
    @ResponseBody
    InfrastructureResponse recover(@RequestParam("subsystem") String subSystem,
        @RequestParam("data_type") String dataType, @RequestParam("path") String path);

    /**
     * 提供pod网络多平面信息
     *
     * @param appName statefulset名称
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/collect/netplane/info")
    @ResponseBody
    InfraResponseWithError<List<NodePodInfo>> getCollectNetPlaneInfo(@RequestParam("appName") String appName);

    /**
     * 提供pod网络多平面信息
     *
     * @param vipInfo VIP信息
     * @return InfrastructureResponse
     */
    @ExterAttack
    @PostMapping("/v1/infra/collect/vip/info")
    @ResponseBody
    InfraResponseWithError<Object> setCollectVipInfo(@RequestBody VipInfo vipInfo);

    /**
     * 基础设施调用k8s接口获取宿主机节点信息
     *
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/collect/node/info")
    @ResponseBody
    InfraResponseWithError<List<NodeDetail>> getInfraNodeInfo();

    /**
     * secret修改接口
     *
     * @param infraConfigMapRequest configMap信息
     * @return InfrastructureResponse
     */
    @ExterAttack
    @PostMapping("/v1/infra/secret/setup")
    @ResponseBody
    InfraResponseWithError<String> updateSecret(@RequestBody InfraConfigMapRequest infraConfigMapRequest);

    /**
     * secret新增接口
     *
     * @param nameSpace 命令空间
     * @param secretMap secretMap
     * @param secretKey secretKey key对象信息
     * @param secretValue secretValue
     * @return InfraResponseWithError
     */
    @ExterAttack
    @PostMapping("/v1/infra/secret/create")
    @ResponseBody
    InfraResponseWithError<String> createSecret(@RequestParam("nameSpace") String nameSpace,
            @RequestParam("secretMap") String secretMap, @RequestParam("secretKey") String secretKey,
            @RequestParam("secretValue") String secretValue);

    /**
     * secret删除接口
     *
     * @param nameSpace 命令空间
     * @param secretMap secretMap
     * @param secretKey secretKey key对象信息
     * @return InfraResponseWithError
     */
    @ExterAttack
    @PostMapping("/v1/infra/secret/delete")
    @ResponseBody
    InfraResponseWithError<String> deleteSecret(@RequestParam("nameSpace") String nameSpace,
            @RequestParam("secretMap") String secretMap, @RequestParam("secretKey") String secretKey);

    /**
     * secret查询接口
     *
     * @param nameSpace 命令空间
     * @param secretName secretName 对象
     * @return InfraResponseWithError
     */
    @ExterAttack
    @GetMapping("/v1/infra/secret/info")
    @ResponseBody
    InfraResponseWithError<JSONArray> getSecret(@RequestParam("nameSpace") String nameSpace,
            @RequestParam("secretName") String secretName);

    /**
     * HA证书更新
     *
     * @param type 类型 update or rollback
     * @param role 角色 primary or standby
     * @return 调用结果
     */
    @ExterAttack
    @PutMapping("/v1/infra/internal/clusters/ha/cert")
    InfraResponseWithError<String> updateHaCert(@RequestParam("role") String role, @RequestParam("type") String type);

    /**
     * 调用基础设施接口查询configMap中某个key的value信息
     *
     * @param nameSpace 命令空间
     * @param configMap configMap
     * @param configKey configMap key对象
     * @return response
     */
    @ExterAttack
    @GetMapping("/v1/infra/configmap/info")
    @ResponseBody
    InfraResponseWithError<List<JSONObject>> getCommonConfValue(@RequestParam("nameSpace") String nameSpace,
            @RequestParam("configMap") String configMap, @RequestParam("configKey") String configKey);

    /**
     * 调用基础设施接口查询configMap中某个key的value信息
     *
     * @param nameSpace 命令空间
     * @param configMap configMap
     * @return response
     */
    @ExterAttack
    @GetMapping("/v1/infra/configmap/info")
    @ResponseBody
    InfraResponseWithError<List<JSONObject>> getCommonConfValue(@RequestParam("nameSpace") String nameSpace,
            @RequestParam("configMap") String configMap);

    /**
     * 调用基础设施接口添加ConfigMap信息
     *
     * @param nameSpace 命名空间
     * @param configMap configMap的名称
     * @param configKey data中的key
     * @param configValue data中的value
     * @return response 接口设置返回信息
     */
    @ExterAttack
    @PostMapping("/v1/infra/configmap/setup")
    @ResponseBody
    InfraResponseWithError<String> setConfigMapInfo(@RequestParam("nameSpace") String nameSpace,
        @RequestParam("configMap") String configMap, @RequestParam("configKey") String configKey,
        @RequestParam("configValue") String configValue);

    /**
     * 调用基础设施接口添加ConfigMap信息
     *
     * @param configMapOperationParam 添加configMap信息请求体
     * @return InfraResponseWithError
     */
    @ExterAttack
    @PostMapping("/v2/infra/configmap/setup")
    @ResponseBody
    InfraResponseWithError<String> setConfigMapInfo(@RequestBody ConfigMapOperationParam configMapOperationParam);

    /**
     * 调用基础设施接口创建ConfigMap的key/value
     *
     * @param nameSpace 命名空间
     * @param configMap configMap的名称
     * @param configKey data中的key
     * @param configValue data中的value
     * @return response 接口设置返回信息
     */
    @ExterAttack
    @PostMapping("/v1/infra/configmap/create")
    @ResponseBody
    InfraResponseWithError<String> createConfigMapInfo(@RequestParam("nameSpace") String nameSpace,
        @RequestParam("configMap") String configMap, @RequestParam("configKey") String configKey,
        @RequestParam("configValue") String configValue);

    /**
     * secret新增接口
     *
     * @param configMapOperationParam 创建configMap请求体
     * @return InfraResponseWithError
     */
    @ExterAttack
    @PostMapping("/v2/infra/configmap/create")
    @ResponseBody
    InfraResponseWithError<String> createConfigMapInfo(@RequestBody ConfigMapOperationParam configMapOperationParam);

    /**
     * 调用基础设施接口删除字段
     *
     * @param nameSpace 命令空间
     * @param configMap configMap
     * @param configKey configMap key对象
     * @return response
     */
    @ExterAttack
    @PostMapping("/v1/infra/configmap/delete")
    @ResponseBody
    InfraResponseWithError<String> deleteConfigMapKey(@RequestParam("nameSpace") String nameSpace,
        @RequestParam("configMap") String configMap, @RequestParam("configKey") String configKey);

    /**
     * 调用基础设施修改service 修改gaussDB/ES IP指向
     *
     * @param infraAddServiceAndModifyComponentIpRequest infraAddServiceAndModifyComponentIpRequest
     * @return response
     */
    @ExterAttack
    @PostMapping("/v1/infra/cluster/operation")
    @ResponseBody
    InfrastructureResponse addServiceAndModifyComponentIp(
        @RequestBody InfraAddServiceAndModifyComponentIpRequest infraAddServiceAndModifyComponentIpRequest);

    /**
     * 删除指定POD
     *
     * @param moduleName 删除pod模块名，目前只支持PM
     * @return InfrastructureResponse
     */
    @ExterAttack
    @PostMapping("/v1/infra/pod/delete")
    @ResponseBody
    InfrastructureResponse deletePod(@RequestParam("moduleName") String moduleName);

    /**
     * 停止指定容器服务
     *
     * @param stateName 停止指定pod
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/designated_pod/stop")
    @ResponseBody
    InfrastructureResponse stopContainer(@RequestParam("stateName") String stateName);

    /**
     * 拉起指定容器服务
     *
     * @param stateName 拉起指定pod
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/designated_pod/recover")
    @ResponseBody
    InfrastructureResponse recoverPod(@RequestParam("stateName") String stateName);

    /**
     * 拉起未启动的所有容器服务
     *
     * @param step om参数
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/external/pod/recover")
    @ResponseBody
    InfrastructureResponse recoverContainer(@RequestParam("step") String step);

    /**
     * 重启爱数容器
     *
     * @return InfrastructureResponse
     */
    @ExterAttack
    @PostMapping("/v1/infra/protectengine-a/restart-all-container")
    @ResponseBody
    InfrastructureResponse restartEngineA();

    /**
     * 基础设施调用k8s接口获取宿主机 容器信息
     *
     * @param appName statefulSet名称
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/internal/pod/info")
    @ResponseBody
    InfraResponseWithError<List<NodePodInfo>> getInfraPodInfo(@RequestParam("appName") String appName);

    /**
     * 基础设施调用k8s接口获取pod和容器信息状态
     *
     * @return InfrastructureResponse
     */
    @ExterAttack
    @GetMapping("/v1/infra/pod/status")
    @ResponseBody
    InfraResponseWithError<List<NodePodInfo>> getInfraContainerInfo();

    /**
     * 调用基础设施接口查询爱数服务状态
     *
     * @return status结果：ready表示成功，failed表示失败，installing表示启动中，uninstalling表示还没有启动完全
     */
    @ExterAttack
    @GetMapping("/v1/infra/service/status")
    @ResponseBody
    InfraResponseWithError<String> getPodServiceStatus();

    /**
     * 调用基础设施接口查询所有节点ip
     *
     * @param endpointName endpointName
     * @return status结果：ready表示成功，failed表示失败，installing表示启动中，uninstalling表示还没有启动完全
     */
    @ExterAttack
    @GetMapping("/v1/infra/collect/endpoints/info")
    @ResponseBody
    InfraResponseWithError<List<String>> getEndpoints(@RequestParam("endpointName") String endpointName);

    /**
     * 调用基础设施接口启用或者禁用SFTP服务
     *
     * @param switchRequest 启用或者禁用（open-启用，close-禁用）
     * @return InfrastructureResponse 结果：success:true or false
     */
    @ExterAttack
    @PostMapping("/v1/infra/internal/sftp/switch")
    @ResponseBody
    InfrastructureResponse switchSftpServer(@RequestBody SwitchRequest switchRequest);

    /**
     * 在common-secret中增加SFTP用户
     *
     * @param sftpUserInfo sftp用户信息
     * @return InfrastructureResponse 结果：success:true or false
     */
    @ExterAttack
    @PostMapping("/v1/infra/internal/sftp/secret/add_user")
    @ResponseBody
    InfrastructureResponse createSftpUserInSecret(@RequestBody SftpUserInfo sftpUserInfo);

    /**
     * 在common-secret中删除SFTP用户
     *
     * @param username 用户名
     * @return InfrastructureResponse 结果：success:true or false
     */
    @ExterAttack
    @DeleteMapping("/v1/infra/internal/sftp/secret/delete_user")
    @ResponseBody
    InfrastructureResponse deleteSftpUserInSecret(@RequestBody SftpUsernameInfo username);

    /**
     * 在common-secret中修改SFTP用户密码
     *
     * @param sftpUserInfo SFTP用户信息
     * @return InfrastructureResponse 结果：success:true or false
     */
    @ExterAttack
    @PostMapping("/v1/infra/internal/sftp/secret/password")
    @ResponseBody
    InfrastructureResponse modifySftpPasswdInSecret(@RequestBody SftpUserInfo sftpUserInfo);

    /**
     * 根据pod IP地址获取节点信息
     *
     * @param podIp 容器虚拟IP
     * @return 节点信息
     */
    @ExterAttack
    @GetMapping("/v1/infra/internal/map/pod/node")
    @ResponseBody
    InfraResponseWithError<NodeControllerInfo> getNodeInfoByPodIp(@RequestParam("podIp") String podIp);

    /**
     * 启动标准备份服务
     *
     * @return 返回信息
     */
    @ExterAttack
    @PostMapping("/v1/infra/protectengine-a/start")
    @ResponseBody
    InfraResponseWithError<String> startStandardBackupService();

    /**
     * 启动回滚标准备份服务
     *
     * @return 返回信息
     */
    @ExterAttack
    @PostMapping("/v1/infra/protectengine-a/init-rollback")
    @ResponseBody
    InfraResponseWithError<String> rollbackStandardBackupService();

    /**
     * 查询回滚标准备份服务结果
     *
     * @return 返回信息
     */
    @ExterAttack
    @GetMapping("/v1/infra/protectengine-a/rollback-init-status")
    @ResponseBody
    InfraResponseWithError<String> queryRollBackStandardBackupServiceStatus();

    /**
     * 调用基础设施重启服务
     *
     * @param certificateTypeInfo 组件类型
     * @return 返回信息
     */
    @ExterAttack
    @PostMapping("/v1/infra/internal/cert/action/replace")
    @ResponseBody
    InfrastructureResponse reboot(@RequestBody CertificateTypeInfo certificateTypeInfo);

    /**
     * 查询pvc文件系统接口
     *
     * @param nameSpace 命名空间名
     * @return pvc文件系统名
     */
    @ExterAttack
    @GetMapping("/v1/infra/internal/pvc/list")
    InfraResponseWithError<List<String>> queryFileSystemNameList(@RequestParam("nameSpace") String nameSpace);

    /**
     * 配置HA校验网络连通性
     *
     * @param request 浮动IP和仲裁网关
     * @return 校验结果
     */
    @ExterAttack
    @PostMapping("/v1/infra/internal/clusters/ha/network-check")
    InfraResponseWithError<String> checkFloatIpAndGateway(@RequestBody NetworkCheckRequest request);

    /**
     * 添加HA成员
     *
     * @param request 浮动IP和仲裁网关
     * @return 调用结果
     */
    @ExterAttack
    @PostMapping("/v1/infra/internal/clusters/ha")
    InfraResponseWithError<String> addHaConfig(@RequestBody AddStandbyHaRequest request);

    /**
     * 移除HA成员
     *
     * @param role 角色
     * @return 调用结果
     */
    @ExterAttack
    @DeleteMapping("/v1/infra/internal/clusters/ha")
    InfraResponseWithError<String> removeHaConfig(@RequestParam("role") String role);

    /**
     * 查询异步操作任务进度接口
     *
     * @param type 查询的操作类型，add、modify、remove、post
     * @return 异步任务进度
     */
    @ExterAttack
    @GetMapping("/v1/infra/internal/clusters/ha/status")
    InfraResponseWithError<ClusterHaStatusResponse> getHaOperationStatus(@RequestParam("type") String type);

    /**
     * HA后置操作接口
     *
     * @param request 请求体
     * @return 请求结果
     */
    @ExterAttack
    @PostMapping("/v1/infra/internal/clusters/ha/post")
    InfraResponseWithError<String> postClusterHa(@RequestBody PostClusterHaRequest request);

    /**
     * 修改HA配置
     *
     * @param updateHaConfigInfraRequest request
     * @return 返回信息
     */
    @ExterAttack
    @PutMapping("/v1/infra/internal/clusters/ha")
    @ResponseBody
    InfraResponseWithError<String> updateHaConfig(@RequestBody UpdateHaConfigInfraRequest updateHaConfigInfraRequest);

    /**
     * 查询日志等级
     *
     * @return logLevelInfos
     */
    @ExterAttack
    @GetMapping("/v1/infra/logs/level/info")
    @ResponseBody
    InfraResponseWithError<LogLevelInfo> getLogLevelInfo();

    /**
     * 校验内部通信网络平面是否连通
     *
     * @param clusterNetworkRequest 请求体
     * @return 请求结果
     */
    @ExterAttack
    @PostMapping("/v1/infra/external/cluster/check_net_work")
    @ResponseBody
    InfraResponseWithError<String> checkInternalCommunicateNetPlane(
            @RequestBody ClusterNetworkRequest clusterNetworkRequest);

    /**
     * 通知om备份成员节点kmc、证书、secret
     *
     * @return 请求结果
     */
    @ExterAttack
    @PostMapping("/v1/infra/external/cluster/backup")
    @ResponseBody
    InfraResponseWithError<String> backupMemberClusterInfo();

    /**
     * 通知om清除备份的成员节点kmc、证书、secret
     *
     * @return 请求结果
     */
    @ExterAttack
    @PostMapping("/v1/infra/external/cluster/backup_clean")
    @ResponseBody
    InfraResponseWithError<String> clearMemberClusterBackupInfo();

    /**
     * 重新生成证书 内部通讯证书和内部数据库证书
     *
     * @param certificateTypeInfo 组件类型
     * @return 返回信息
     */
    @ExterAttack
    @PostMapping("/v1/infra/internal/cert/action/generate")
    @ResponseBody
    InfraResponseWithError<List<FileSyncEntity>> regenerateInnerCert(@RequestBody CertificateTypeInfo
        certificateTypeInfo);
}
