/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster;

import openbackup.system.base.common.annotation.Routing;
import openbackup.system.base.common.constants.BackupBaseClusterInfo;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.JobReportBo;
import openbackup.system.base.common.model.job.request.QueryReportJobsRequest;
import openbackup.system.base.common.model.repository.tape.TapeSetDetailResponse;
import openbackup.system.base.sdk.accesspoint.model.CleanRemoteRequest;
import openbackup.system.base.sdk.alarm.bo.AlarmVO;
import openbackup.system.base.sdk.alarm.model.AlarmInfo;
import openbackup.system.base.sdk.alarm.model.ClusterAlarmsInfo;
import openbackup.system.base.sdk.alarm.model.NodeAlarmInfo;
import openbackup.system.base.sdk.alarm.model.PerAlarmQueryParam;
import openbackup.system.base.sdk.archive.model.ArchiveMsg;
import openbackup.system.base.sdk.auth.UserAuthRequest;
import openbackup.system.base.sdk.auth.UserDetail;
import openbackup.system.base.sdk.auth.UserRequest;
import openbackup.system.base.sdk.auth.UserResponse;
import openbackup.system.base.sdk.auth.model.response.UserPageListResponse;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.cluster.model.CheckInfo;
import openbackup.system.base.sdk.cluster.model.ClusterCapacityInfo;
import openbackup.system.base.sdk.cluster.model.ClusterComponentPwdInfo;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.ClusterJobsInfo;
import openbackup.system.base.sdk.cluster.model.ClusterRelationRequest;
import openbackup.system.base.sdk.cluster.model.ClusterResourcesInfo;
import openbackup.system.base.sdk.cluster.model.ClusterSlaComplianceInfo;
import openbackup.system.base.sdk.cluster.model.ClusterStorageNodeVo;
import openbackup.system.base.sdk.cluster.model.ClusterStorageTendencyInfo;
import openbackup.system.base.sdk.cluster.model.ClusterUserResponse;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.sdk.cluster.model.DmeRemovePairRequest;
import openbackup.system.base.sdk.cluster.model.LocalClusterRequest;
import openbackup.system.base.sdk.cluster.model.MemberClusterInfo;
import openbackup.system.base.sdk.cluster.model.OperateComponentRequest;
import openbackup.system.base.sdk.cluster.model.ProductStorageInfo;
import openbackup.system.base.sdk.cluster.model.SetupConfigMapRequest;
import openbackup.system.base.sdk.cluster.model.SoftwareVersion;
import openbackup.system.base.sdk.cluster.model.StorageCapacitySummaryVo;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.cluster.model.StorageUserAuthRelationStorageVo;
import openbackup.system.base.sdk.cluster.model.SyncComponentInfoRequest;
import openbackup.system.base.sdk.cluster.model.SyncComponentIpRequest;
import openbackup.system.base.sdk.cluster.model.TargetClusterInfoVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterRequest;
import openbackup.system.base.sdk.cluster.model.TokenResponse;
import openbackup.system.base.sdk.cluster.model.ha.AddStandbyHaRequest;
import openbackup.system.base.sdk.cluster.model.ha.CheckStandbyNetworkRequest;
import openbackup.system.base.sdk.cluster.model.ha.HaOperationStatusResponse;
import openbackup.system.base.sdk.cluster.model.ha.PostHaRequest;
import openbackup.system.base.sdk.cluster.model.ha.UpdateStandbyHaConfigRequest;
import openbackup.system.base.sdk.cluster.model.storage.StoragePolicyRelationRequest;
import openbackup.system.base.sdk.cluster.netplane.NetPlaneInfoReq;
import openbackup.system.base.sdk.cluster.netplane.NetworkPlane;
import openbackup.system.base.sdk.cluster.request.BackupTaskRequest;
import openbackup.system.base.sdk.cluster.request.RecentJobQueryReq;
import openbackup.system.base.sdk.cluster.request.RecoveryTaskRequest;
import openbackup.system.base.sdk.dee.model.FineGrainedRestore;
import openbackup.system.base.sdk.dee.model.ModifyEsClusterReq;
import openbackup.system.base.sdk.exportfile.model.LogLevelDto;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.security.exterattack.ExterAttack;

import feign.Headers;
import feign.Param;
import feign.QueryMap;
import feign.RequestLine;

import org.springframework.web.bind.annotation.RequestBody;

import java.net.URI;
import java.util.List;
import java.util.Map;

/**
 * Target Cluster Rest Api
 *
 * @author p30001902
 * @since 2020-07-21
 */
@Routing(destinationIp = "#{uri.host}", requires = {"#{uri.port}=30068"}, onNetPlane = "replication")
public interface TargetClusterRestApi {
    /**
     * Get target cluster token
     *
     * @param uri target url addr
     * @param request request
     * @return token
     */
    @ExterAttack
    @RequestLine("GET /v1/auth/token")
    TokenResponse getToken(URI uri, @RequestBody UserAuthRequest request);

    /**
     * get target cluster users info
     *
     * @param uri target url addr
     * @param token token
     * @return 用户列表
     */
    @ExterAttack
    @RequestLine("GET /v1/users?startIndex=1&pageSize=64")
    @Headers({"x-auth-token: {token}"})
    ClusterUserResponse<UserDetail> getAllClusterUser(URI uri, @Param("token") String token);

    /**
     * 创建用户
     *
     * @param uri uri
     * @param token token
     * @param user user
     * @return UserDetail
     */
    @ExterAttack
    @RequestLine("POST /v1/users")
    @Headers({"x-auth-token: {token}"})
    UserDetail createClusterUser(URI uri, @Param("token") String token, @RequestBody UserRequest user);

    /**
     * Get target cluster info
     *
     * @param uri target url addr
     * @param token token
     * @return TargetClusterInfo obj
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/details")
    @Headers({"x-auth-token: {token}"})
    ClusterDetailInfo getTargetClusterInfo(URI uri, @Param("token") String token);

    /**
     * Query cluster status
     *
     * @param uri target url addr
     * @param token token
     * @param sourceEsn source cluster esn
     * @return status
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/status?sourceEsn={sourceEsn}")
    @Headers({"x-auth-token: {token}"})
    ClusterEnum.StatusEnum getClusterStatus(URI uri, @Param("token") String token,
        @Param("sourceEsn") String sourceEsn);

    /**
     * Verify target cluster
     *
     * @param uri target url addr
     * @param request request body
     * @param token token
     * @return Void obj
     */
    @RequestLine("POST /v1/clusters/target/relation")
    @Headers({"x-auth-token: {token}"})
    Void manageTargetClusterRelation(URI uri, @Param("token") String token,
        @RequestBody ClusterRelationRequest request);

    /**
     * 指定某个被管理集群作为管理集群
     *
     * @param uri target url addr
     * @param token token
     * @param clusterId 指定操作对应的被管理集群的ID
     * @param request request附带参数，是否同步到管理集群远端，如果需要同步，则需要用户名，密码，集群名称
     * @return void
     */
    @RequestLine("PUT /v1/multi-clusters/{clusterId}/action/grant-to-manager")
    @Headers({"x-auth-token: {token}"})
    Void grantToManager(URI uri, @Param("token") String token, @Param(value = "clusterId") Integer clusterId,
        @RequestBody LocalClusterRequest request);

    /**
     * 取消管理集群
     *
     * @param uri target url addr
     * @param token token
     * @param clusterId 取消操作对应的管理集群的ID
     * @param shouldSyncToRemote 取消操作是否同步到管理集群远端
     * @return void
     */
    @RequestLine("PUT /v1/multi-clusters/{clusterId}/action/revoke-manager?syncToRemote={syncToRemote}")
    @Headers({"x-auth-token: {token}"})
    Void revokeManager(URI uri, @Param("token") String token, @Param(value = "clusterId") Integer clusterId,
        @Param("syncToRemote") Boolean shouldSyncToRemote);

    /**
     * 同步本地的所有被管理集群到本地的所有管理集群
     *
     * @param uri target url addr
     * @param token token
     * @return void
     */
    @RequestLine("POST /v1/multi-clusters/action/sync")
    @Headers({"x-auth-token: {token}"})
    Void syncManagedClustersToManageClusters(URI uri, @Param("token") String token);

    /**
     * add the target cluster to another manager cluster
     *
     * @param req add request
     * @param uri of the another manager cluster
     * @param token of the another manage cluster
     * @return 集群id
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters")
    @Headers({"x-auth-token: {token}"})
    int syncAddTargetClusterToRemote(URI uri, @Param("token") String token, @RequestBody TargetClusterRequest req);

    /**
     * sync update the target cluster to another manager cluster
     *
     * @param uri of the another manager cluster
     * @param token of the another manage cluster
     * @param clusterId clusterId
     * @param req sync update request
     * @return Void obj
     */
    @RequestLine("PUT /v1/clusters/{clusterId}")
    @Headers({"x-auth-token: {token}"})
    Void syncUpdateTargetClusterToRemote(URI uri, @Param("token") String token, @Param("clusterId") int clusterId,
        @RequestBody TargetClusterRequest req);

    /**
     * sync update the target cluster to another manager cluster
     *
     * @param uri of the another manager cluster
     * @param token of the another manage cluster
     * @param clusterId clusterId
     * @param shouldForceDelete whether force deleting
     * @param shouldDeleteInLocal whether only local deleted
     * @return Void obj
     */
    @RequestLine("DELETE /v1/clusters/{clusterId}?forceDelete={forceDelete}&deleteInLocal={deleteInLocal}")
    @Headers({"x-auth-token: {token}"})
    Void syncDeleteTargetClusterToRemote(URI uri, @Param("token") String token, @Param("clusterId") int clusterId,
        @Param("forceDelete") boolean shouldForceDelete, @Param("deleteInLocal") boolean shouldDeleteInLocal);

    /**
     * Get all the target cluster info of another manager cluster
     *
     * @param uri of the another manager cluster
     * @param token of the another manage cluster
     * @return PageListResponse<ClustersInfoVo> list of target cluster info of another manager cluster
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<ClustersInfoVo> getAllTargetsOfAnotherMngClusterInfo(URI uri, @Param("token") String token);

    /**
     * 获取目标集群资源信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterResourcesInfo
     */
    @ExterAttack
    @RequestLine("GET v1/resource/protection/summary")
    @Headers({"x-auth-token: {token}"})
    ClusterResourcesInfo getClusterResources(URI uri, @Param("token") String token);

    /**
     * 浏览副本中文件和目录信息
     *
     * @param uri uri
     * @param token token
     * @param copyId copy id
     * @param queryParams queryParams
     * @return 副本文件和目录信息
     */
    @ExterAttack
    @RequestLine("GET /v2/copies/{copyId}/catalogs")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<FineGrainedRestore> listCopyCatalogs(URI uri, @Param("token") String token,
        @Param("copyId") String copyId, @QueryMap Map<String, Object> queryParams);

    /**
     * 浏览副本中文件和目录信息
     *
     * @param uri uri
     * @param token token
     * @param copyId copy id
     * @param queryParams queryParams
     * @return 副本文件和目录信息
     */
    @ExterAttack
    @RequestLine("GET /v2/copies/{copyId}/catalogs-name")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<FineGrainedRestore> listCopyCatalogsName(URI uri, @Param("token") String token,
        @Param("copyId") String copyId, @QueryMap Map<String, Object> queryParams);

    /**
     * 获取目标集群任务信息
     *
     * @param uri uri
     * @param token token
     * @param queryParams query参数
     * @return ClusterJobsInfo
     */
    @ExterAttack
    @RequestLine("GET v1/jobs/summary")
    @Headers({"x-auth-token: {token}"})
    ClusterJobsInfo getClusterJobs(URI uri, @Param("token") String token, @QueryMap Map<String, Object> queryParams);

    /**
     * 获取目标集群任务详细信息
     *
     * @param uri uri
     * @param token token
     * @param queryJobRequest queryJobRequest
     * @return JobReportBo
     */
    @ExterAttack
    @RequestLine("POST /v1/jobs/action/report")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<JobReportBo> queryReportJobs(URI uri, @Param("token") String token,
        @RequestBody QueryReportJobsRequest queryJobRequest);

    /**
     * 获取目标集群告警信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterAlarmsInfo
     */
    @ExterAttack
    @RequestLine("GET /v1/alarms/count")
    @Headers({"x-auth-token: {token}"})
    ClusterAlarmsInfo getClusterAlarms(URI uri, @Param("token") String token);

    /**
     * 获取目标节点告警信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterAlarmsInfo
     */
    @ExterAttack
    @RequestLine("GET /v2/alarms/node/count")
    @Headers({"x-auth-token: {token}"})
    ClusterAlarmsInfo getNodeAlarmsCount(URI uri, @Param("token") String token);

    /**
     * 获取目标集群所有节点的告警信息
     *
     * @param uri uri
     * @param token token
     * @param nodeName 节点名称
     * @return 节点告警统计数据
     */
    @ExterAttack
    @RequestLine("GET /v2/alarms/nodes/info?nodeName={nodeName}")
    @Headers({"x-auth-token: {token}"})
    List<NodeAlarmInfo> getNodesAlarm(URI uri, @Param("token") String token, @Param("nodeName") String nodeName);

    /**
     * 获取目标集群容量信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterCapacityInfo
     */
    @ExterAttack
    @RequestLine("GET v1/clusters/capacity")
    @Headers({"x-auth-token: {token}"})
    ClusterCapacityInfo getClusterCapacity(URI uri, @Param("token") String token);

    /**
     * 获取目标集群容量信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterCapacityInfo
     */
    @ExterAttack
    @RequestLine("GET v1/clusters/capacity/tendency")
    @Headers({"x-auth-token: {token}"})
    ClusterStorageTendencyInfo getClusterStorageTendency(URI uri, @Param("token") String token);

    /**
     * 获取目标集群所有成员信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterCapacityInfo
     */
    @ExterAttack
    @RequestLine("GET v1/clusters/backup/members")
    @Headers({"x-auth-token: {token}"})
    List<MemberClusterInfo> getBackupClusterMembers(URI uri, @Param("token") String token);

    /**
     * 获取目标集群Sla信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterSlaComplianceInfo
     */
    @ExterAttack
    @RequestLine("GET v1/protected-objects/sla-compliance")
    @Headers({"x-auth-token: {token}"})
    ClusterSlaComplianceInfo getClusterSlaCompliance(URI uri, @Param("token") String token);

    /**
     * 获取目标集群用户信息
     *
     * @param uri uri
     * @param token token
     * @param userId userId
     * @return UserDetail
     */
    @ExterAttack
    @RequestLine("GET /v1/users/{userId}")
    @Headers({"x-auth-token: {token}"})
    UserDetail getClusterUserInfoById(URI uri, @Param("token") String token, @Param("userId") String userId);

    /**
     * 校验目标端额度
     *
     * @param uri uri
     * @param token token
     * @param userId userId
     * @param resourceId resourceId
     * @param taskType taskType
     * @return Void obj
     */
    @ExterAttack
    @RequestLine("GET /v1/users/quota/action/check?userId={userId}&resourceId={resourceId}&taskType={taskType}")
    @Headers({"x-auth-token: {token}"})
    Void checkTargetQuota(URI uri, @Param("token") String token, @Param("userId") String userId,
        @Param("resourceId") String resourceId, @Param("taskType") String taskType);

    /**
     * 查看备份节点内部通信网络平面
     *
     * @param uri uri
     * @param token token
     * @param infraNetPlaneIp infraNetPlaneIp
     * @return 备份节点内部通信网络平面
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/internal-communicate-netplane?infraNetPlaneIp={infraNetPlaneIp}")
    @Headers({"x-auth-token: {token}"})
    String getClusterInternalCommunicateNetPlane(URI uri, @Param("token") String token,
        @Param("infraNetPlaneIp") String infraNetPlaneIp);

    /**
     * Add internal net plane relation
     *
     * @param uri target url addr
     * @param token token
     * @param netPlaneInfo netPlaneInfo
     * @return Void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/backup/relate/netplane")
    @Headers({"x-auth-token: {token}"})
    Void addInternalNetPlaneRelation(URI uri, @Param("token") String token, @RequestBody NetPlaneInfoReq netPlaneInfo);

    /**
     * UPDATE internal net plane relation
     *
     * @param uri target url addr
     * @param token token
     * @param netPlaneInfo netPlaneInfo
     * @return Void
     */
    @ExterAttack
    @RequestLine("PUT /v1/clusters/backup/relate/netplane")
    @Headers({"x-auth-token: {token}"})
    Void updateInternalNetPlaneRelation(URI uri, @Param("token") String token,
        @RequestBody NetPlaneInfoReq netPlaneInfo);

    /**
     * Backup cluster provide dm storage info
     *
     * @param uri target url addr
     * @param token token
     * @param targetClusterInfoVo targetClusterInfoVo
     * @return Void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/backup/local-storage/action/collect")
    @Headers({"x-auth-token: {token}"})
    Void requestBackupClusterDmStorageInfo(URI uri, @Param("token") String token,
        @RequestBody TargetClusterInfoVo targetClusterInfoVo);

    /**
     * Main cluster save dm storage info
     *
     * @param uri target url addr
     * @param token token
     * @param dmStorageInfo dm storage info
     * @return Void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/backup/local-storage/action/save")
    @Headers({"x-auth-token: {token}"})
    Void responseBackupClusterDmStorageInfo(URI uri, @Param("token") String token,
        @RequestBody List<ProductStorageInfo> dmStorageInfo);

    /**
     * 同步内部组件密码接口
     *
     * @param uri target url addr
     * @param token token
     * @param syncComponentInfoRequest syncComponentPwdRequest
     * @return Void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/backup/component/sync-info")
    @Headers({"x-auth-token: {token}"})
    Void syncComponentInfo(URI uri, @Param("token") String token,
        @RequestBody SyncComponentInfoRequest syncComponentInfoRequest);

    /**
     * 功能说明 校验成员节点满足度
     *
     * @param uri uri
     * @param token token
     * @param version 版本号
     * @param netPlaneName 网络平面名称
     * @param deployType 部署类型
     * @return CheckInfo 检查信息
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/backup/check?version={version}&netPlaneName={netPlaneName}&deployType={deployType}")
    @Headers({"x-auth-token: {token}"})
    CheckInfo checkBackupCluster(URI uri, @Param("token") String token, @Param("version") String version,
        @Param("netPlaneName") String netPlaneName, @Param("deployType") String deployType);

    /**
     * 功能说明 通知成员节点修改GaussDB ES映射接口
     *
     * @param uri uri
     * @param token token
     * @param syncComponentIpRequest 组件信息列表
     * @return Void void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/backup/component/sync-ip")
    @Headers({"x-auth-token: {token}"})
    Void syncComponentIp(URI uri, @Param("token") String token,
        @RequestBody SyncComponentIpRequest syncComponentIpRequest);

    /**
     * 功能说明 通知成员节点启动/停止组件接口
     *
     * @param uri uri
     * @param token token
     * @param operateComponentRequest 组件信息
     * @return Void void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/backup/component/operate")
    @Headers({"x-auth-token: {token}"})
    Void operateComponent(URI uri, @Param("token") String token,
        @RequestBody OperateComponentRequest operateComponentRequest);

    /**
     * 功能说明 通知成员节点设置集群角色
     *
     * @param uri uri
     * @param token token
     * @param setupConfigMapRequest 集群设置请求体
     * @return Void void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/backup/config-map")
    @Headers({"x-auth-token: {token}"})
    Void setUpClusterConfigMap(URI uri, @Param("token") String token,
        @RequestBody SetupConfigMapRequest setupConfigMapRequest);

    /**
     * 功能说明 查询集群信息
     *
     * @param uri uri
     * @param token token
     * @return Void void
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/backup/cluster-info")
    @Headers({"x-auth-token: {token}"})
    BackupBaseClusterInfo queryClusterInfo(URI uri, @Param("token") String token);

    /**
     * 根据网络平面名称查询网络平面信息
     *
     * @param uri 目标url地址
     * @param token token
     * @param netPlaneName 网络平面名称
     * @return 网络平面详情
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/backup/netplane/{netPlaneName}")
    @Headers({"x-auth-token: {token}"})
    NetworkPlane getNetPlaneByName(URI uri, @Param("token") String token,
        @Param(value = "netPlaneName") String netPlaneName);

    /**
     * 校验备节点浮动IP和仲裁网关
     *
     * @param uri 目标url地址
     * @param token token
     * @param request 请求体
     * @return void void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/ha/network-check")
    @Headers({"x-auth-token: {token}"})
    Void checkFloatIpAndGateway(URI uri, @Param("token") String token, @RequestBody CheckStandbyNetworkRequest request);

    /**
     * 查询网络平面信息
     *
     * @param uri 目标url地址
     * @param token token
     * @param appName appName
     * @return 网络平面信息列表
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/collect/netplane/info?appName={appName}")
    @Headers({"x-auth-token: {token}"})
    String getCollectNetPlaneInfo(URI uri, @Param("token") String token, @Param("appName") String appName);

    /**
     * 查询网络平面信息
     *
     * @param uri 目标url地址
     * @param token token
     * @return 网络平面信息列表
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/backup/netplane")
    @Headers({"x-auth-token: {token}"})
    NetPlaneInfoReq getBackupNetPlaneInfo(URI uri, @Param("token") String token);

    /**
     * 添加备节点HA配置
     *
     * @param uri 目标url地址
     * @param token token
     * @param request 请求体
     * @return void void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/ha/standby")
    @Headers({"x-auth-token: {token}"})
    Void addStandbyHaConfig(URI uri, @Param("token") String token, @RequestBody AddStandbyHaRequest request);

    /**
     * 移除备节点HA配置
     *
     * @param uri 目标url地址
     * @param token token
     * @return void void
     */
    @ExterAttack
    @RequestLine("DELETE /v1/clusters/ha/standby")
    @Headers({"x-auth-token: {token}"})
    Void removeStandbyHaConfig(URI uri, @Param("token") String token);

    /**
     * HA配置请求执行后置任务接口
     *
     * @param uri 目标url地址
     * @param token token
     * @param request 请求体
     * @return void void
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/ha/post")
    @Headers({"x-auth-token: {token}"})
    Void postClusterHa(URI uri, @Param("token") String token, @RequestBody PostHaRequest request);

    /**
     * 查询HA后置任务进度接口
     *
     * @param uri 目标url地址
     * @param token token
     * @param type 任务类型
     * @return 任务进度
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/ha/status?type={type}")
    @Headers({"x-auth-token: {token}"})
    HaOperationStatusResponse getHaOperationStatus(URI uri, @Param("token") String token, @Param("type") String type);

    /**
     * 修改备节点HA配置
     *
     * @param uri uri
     * @param token token
     * @param request updateStandbyHaConfigRequest
     */
    @ExterAttack
    @RequestLine("PUT /v1/clusters/ha/standby")
    @Headers({"x-auth-token: {token}"})
    void updateStandbyHaConfig(URI uri, @Param("token") String token,
        @RequestBody UpdateStandbyHaConfigRequest request);

    /**
     * 查询聚类容量
     *
     * @param uri uri
     * @param token token
     * @return 聚类容量
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/capacity")
    @Headers({"x-auth-token: {token}"})
    ClusterCapacityInfo getClusterStorage(URI uri, @Param("token") String token);

    /**
     * 查询聚类容量
     *
     * @param uri uri
     * @param token token
     * @param esnList ens列表
     * @return 聚类容量
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/nodes/capacity?esnList={esnList}")
    @Headers({"x-auth-token: {token}"})
    List<ClusterCapacityInfo> getNodesCapacity(URI uri, @Param("token") String token,
        @Param("esnList") List<String> esnList);

    /**
     * 查询容量预测
     *
     * @param uri uri
     * @param token token
     * @param esnList ens列表
     * @return 容量预测
     */
    @ExterAttack
    @RequestLine("GET v1/clusters/nodes/capacity/tendency?esnList={esnList}")
    @Headers({"x-auth-token: {token}"})
    List<ClusterStorageTendencyInfo> getNodesCapacityTendency(URI uri, @Param("token") String token,
        @Param("esnList") List<String> esnList);

    /**
     * 查询备份软件信息
     *
     * @param uri uri
     * @param token token
     * @return 查询备份软件信息
     */
    @ExterAttack
    @RequestLine("GET v1/version")
    @Headers({"x-auth-token: {token}"})
    SoftwareVersion querySoftwareNameAndVersion(URI uri, @Param("token") String token);

    /**
     * 删除导出文件
     *
     * @param uri uri
     * @param token token
     * @param id 文件记录id
     */
    @ExterAttack
    @RequestLine("DELETE v1/export-files/{id}")
    @Headers({"x-auth-token: {token}"})
    void deleteExportFiles(URI uri, @Param("token") String token, @Param("id") String id);

    /**
     * 分页查询告警
     *
     * @param uri uri
     * @param token token
     * @param queryParams 查询参数
     * @return 告警数据
     */
    @ExterAttack
    @RequestLine("GET v2/alarms")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<AlarmInfo> getNodeAlarmPage(URI uri, @Param("token") String token,
        @QueryMap Map<String, Object> queryParams);

    /**
     * 根据entityId查询告警
     *
     * @param uri uri
     * @param token token
     * @param param param
     * @return 告警数据
     */
    @ExterAttack
    @RequestLine("POST v1/alarms/detail")
    @Headers({"x-auth-token: {token}"})
    AlarmVO getAlarm(URI uri, @Param("token") String token, @RequestBody PerAlarmQueryParam param);

    /**
     * 根据告警id清除告警
     *
     * @param uri uri
     * @param token token
     * @param param 告警id
     * @return AlarmVO
     */
    @ExterAttack
    @RequestLine("PUT v1/alarms/action/clear")
    @Headers({"x-auth-token: {token}"})
    AlarmVO clearAlarmsByEntityId(URI uri, @Param("token") String token, @RequestBody PerAlarmQueryParam param);

    /**
     * 调用成员集群进行管理数据恢复
     *
     * @param uri uri
     * @param token token
     * @param taskRequest task
     */
    @ExterAttack
    @RequestLine("POST v1/sysbackup/recovery/submit")
    @Headers({"x-auth-token: {token}"})
    void submitRecoverySubTask(URI uri, @Param("token") String token, @RequestBody RecoveryTaskRequest taskRequest);

    /**
     * 调用成员集群进行管理数据备份
     *
     * @param uri uri
     * @param token token
     * @param backupTaskRequest task
     */
    @ExterAttack
    @RequestLine("POST v1/sysbackup/submit")
    @Headers({"x-auth-token: {token}"})
    void submitBackupSubTask(URI uri, @Param("token") String token, @RequestBody BackupTaskRequest backupTaskRequest);

    /**
     * 调用成员节点删除dme复制pair
     *
     * @param uri uri
     * @param token token
     * @param dmeRemovePairRequest DmeRemovePairRequest
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/replicate/removepair/dispatch")
    @Headers({"x-auth-token: {token}"})
    void dispatchRemoveReplicationPair(URI uri, @Param("token") String token,
        @RequestBody DmeRemovePairRequest dmeRemovePairRequest);

    /**
     * 调用成员节点清除复制链路
     *
     * @param uri uri
     * @param token token
     * @param cleanRemoteRequest CleanRemoteRequest
     */
    @ExterAttack
    @RequestLine("POST /v1/clusters/replicate/cleanremote/dispatch")
    @Headers({"x-auth-token: {token}"})
    void dispatchCleanRemote(URI uri, @Param("token") String token, @RequestBody CleanRemoteRequest cleanRemoteRequest);

    /**
     * 获取成员集群详细信息
     *
     * @param uri uri
     * @param token token
     * @return ClusterDetailInfo 成员集群详细信息
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/backup/member/detail")
    @Headers({"x-auth-token: {token}"})
    ClusterDetailInfo getMemberClusterDetail(URI uri, @Param("token") String token);

    /**
     * 删除LAN-FREE配置
     *
     * @param uri uri
     * @param token token
     * @param resourceId 资源ID
     */
    @ExterAttack
    @RequestLine("DELETE /v1/host-agent/{agentId}/lan-free/configuration")
    @Headers({"x-auth-token: {token}"})
    void deleteLanFreeConfig(URI uri, @Param("token") String token, @Param("agentId") String resourceId);

    /**
     * 查询日志等级
     *
     * @param uri uri
     * @param token token
     * @return 日志等级
     */
    @ExterAttack
    @RequestLine("GET /v1/logs/level/info")
    @Headers({"x-auth-token: {token}"})
    LogLevelDto getLogLevelInfo(URI uri, @Param("token") String token);

    /**
     * 查询所有的节点
     *
     * @param uri uri
     * @param clusterId 节点id
     * @param token token
     * @return 节点信息
     */
    @ExterAttack
    @RequestLine("GET /v1/clusters/{clusterId}/nodes")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<ClusterStorageNodeVo> queryClusterNodes(URI uri, @Param("clusterId") Integer clusterId,
        @Param("token") String token);

    /**
     * 归档任务下发到成员集群
     *
     * @param uri uri
     * @param token token
     * @param msg ArchiveMsg
     */
    @ExterAttack
    @RequestLine("POST /v1/archive/dispatch")
    @Headers({"x-auth-token: {token}"})
    void dispatchArchive(URI uri, @Param("token") String token, @RequestBody ArchiveMsg msg);

    /**
     * 更新内部组件密码
     *
     * @param uri target uri
     * @param token token
     * @param clusterComponentPwdInfo 内部组件key和password值
     */
    @ExterAttack
    @RequestLine("PUT /v1/security/component-password/member")
    @Headers({"x-auth-token: {token}"})
    void updateComponentPassword(URI uri, @Param("token") String token,
        @RequestBody List<ClusterComponentPwdInfo> clusterComponentPwdInfo);

    /**
     * 查询磁带库详情
     *
     * @param uri uri
     * @param token token
     * @param mediaSetId mediaSetId
     * @return TapeSetDetailResponse 磁带库详情
     */
    @ExterAttack
    @RequestLine("GET /v1/tape-library/media-sets/{mediaSetId}")
    @Headers({"x-auth-token: {token}"})
    TapeSetDetailResponse getTapeSetDetail(URI uri, @Param("token") String token,
        @Param(("mediaSetId")) String mediaSetId);

    /**
     * 获取性能开关
     *
     * @param uri uri
     * @param token token
     * @return 开启状态
     */
    @ExterAttack
    @RequestLine("GET /v1/cluster/performance/config")
    @Headers({"x-auth-token: {token}"})
    boolean getPerformanceConfig(URI uri, @Param("token") String token);

    /**
     * 转发
     *
     * @param uri uri
     * @param token token
     * @param copyId copyID
     */
    @ExterAttack
    @RequestLine("POST /v1/copies/{copyId}/action/create-index")
    @Headers({"x-auth-token: {token}"})
    void createIndex(URI uri, @Param("token") String token, @Param("copyId") String copyId);

    /**
     * 备节点上报es IP
     *
     * @param uri uri
     * @param token token
     * @param request 上报请求体
     * @return 上报是否成功
     */
    @ExterAttack
    @RequestLine("PUT /v1/clusters/ha/elasticsearch/config")
    @Headers({"x-auth-token: {token}"})
    boolean standbyReportEsIp(URI uri, @Param("token") String token, @RequestBody ModifyEsClusterReq request);

    /**
     * 获取目标集群备份存储容量统计信息
     *
     * @param uri uri
     * @param token token
     * @param isAllCluster 是否查询所有集群
     * @return 备份存储容量统计信息
     */
    @ExterAttack
    @RequestLine("GET /v1/multi-clusters/backup/capacity/summary?isAllCluster={isAllCluster}")
    @Headers({"x-auth-token: {token}"})
    List<StorageCapacitySummaryVo> getMultiClusterBackupCapacitySummary(URI uri, @Param("token") String token,
        @Param("isAllCluster") boolean isAllCluster);

    /**
     * 获取目标集群复制存储容量统计信息
     *
     * @param uri uri
     * @param token token
     * @param isAllCluster 是否查询所有集群
     * @return 复制存储容量统计信息
     */
    @ExterAttack
    @RequestLine("GET /v1/multi-clusters/replication/capacity/summary?isAllCluster={isAllCluster}")
    @Headers({"x-auth-token: {token}"})
    List<StorageCapacitySummaryVo> getMultiClusterReplicationCapacitySummary(URI uri, @Param("token") String token,
        @Param("isAllCluster") boolean isAllCluster);

    /**
     * 获取目标集群归档存储容量统计信息
     *
     * @param uri uri
     * @param token token
     * @param isAllCluster 是否查询所有集群
     * @return 归档存储容量统计信息
     */
    @ExterAttack
    @RequestLine("GET /v1/multi-clusters/archive/capacity/summary?isAllCluster={isAllCluster}")
    @Headers({"x-auth-token: {token}"})
    List<StorageCapacitySummaryVo> getMultiClusterArchiveCapacitySummary(URI uri, @Param("token") String token,
        @Param("isAllCluster") boolean isAllCluster);

    /**
     * 查询外部集群备份存储单元组信息
     *
     * @param uri uri
     * @param token token
     * @param id 备份存储单元组uuid
     * @return 备份存储单元组详情
     */
    @ExterAttack
    @RequestLine("GET /v1/storages/nas/distribution/{id}")
    @Headers({"x-auth-token: {token}"})
    NasDistributionStorageDetail getTargetStorage(URI uri, @Param("token") String token, @Param("id") String id);

    /**
     * 查询外部集群Pod状态
     *
     * @param uri uri
     * @param token token
     * @return 备份存储单元组详情
     */
    @ExterAttack
    @RequestLine("GET /v1/inspect/service/status")
    @Headers({"x-auth-token: {token}"})
    InfraResponseWithError<List<NodePodInfo>> checkPodStatus(URI uri, @Param("token") String token);

    /**
     * 获取目标集群最近任务列表
     *
     * @param uri uri
     * @param token token
     * @param recentJobQueryReq recentJobQueryReq
     * @return ClusterJobsInfo
     */
    @ExterAttack
    @RequestLine("GET /v1/jobs")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<JobBo> getClusterJobsList(URI uri, @Param("token") String token,
                                               @QueryMap RecentJobQueryReq recentJobQueryReq);

    /**
     * 添加策略存储单元关联关系
     *
     * @param uri URI
     * @param token String
     * @param policyId String
     * @param request StoragePolicyRelationRequest
     * @return Void
     */
    @ExterAttack
    @RequestLine("POST /v1/storage-policy-relation/policies/{policyId}")
    @Headers({"x-auth-token: {token}"})
    Void addStoragePolicyRelation(URI uri, @Param("token") String token, @Param("policyId") String policyId,
        @RequestBody StoragePolicyRelationRequest request);

    /**
     * 根据策略ID删除策略存储单元关联关系
     *
     * @param uri URI
     * @param token String
     * @param policyId String
     * @param request StoragePolicyRelationRequest
     * @return Void
     */
    @ExterAttack
    @RequestLine("DELETE /v1/storage-policy-relation/policies/{policyId}")
    @Headers({"x-auth-token: {token}"})
    Void deleteStoragePolicyRelationByPolicyId(URI uri, @Param("token") String token,
        @Param("policyId") String policyId, @RequestBody StoragePolicyRelationRequest request);

    /**
     * 查询指定对端用户的存储单元信息
     *
     * @param uri uri
     * @param token token
     * @param userId userId
     * @param authType authType
     * @return PageListResponse 返回结果
     */
    @ExterAttack
    @RequestLine("GET /v1/storage-user-auths/users/{userId}?authType={authType}")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<StorageUserAuthRelationStorageVo> getRemoteStorageUnitInfo(URI uri, @Param("token") String token,
        @Param("userId") String userId, @Param("authType") int authType);

    /**
     * 查询指定目标集群的存储单元信息
     *
     * @param uri uri
     * @param token token
     * @param queryParam queryParam
     * @param pageNo pageNo
     * @param pageSize pageSize
     * @return PageListResponse 返回结果
     */
    @ExterAttack
    @RequestLine("GET /v1/storage-units?pageNo={pageNo}&pageSize={pageSize}")
    @Headers({"x-auth-token: {token}"})
    PageListResponse<StorageUnitVo> getStorageUnitInfo(URI uri, @Param("token") String token,
        @QueryMap Map<String, String> queryParam, @Param("pageNo") int pageNo, @Param("pageSize") int pageSize);

    /**
     * 查询指定复制目标集群所有数据保护管理员列表
     *
     * @param uri uri
     * @param token token
     * @return PageListResponse 返回结果
     */
    @ExterAttack
    @RequestLine("GET /v1/multiple-storage-pool/remote-dp-user-info")
    @Headers({"x-auth-token: {token}"})
    UserPageListResponse<UserResponse> getAllDp(URI uri, @Param("token") String token);

    /**
     * 校验指定对端用户的存储单元信息
     *
     * @param uri uri
     * @param token token
     * @param storageType storageType
     * @param storageId storageId
     * @param resourceId resourceId
     */
    @ExterAttack
    @RequestLine("GET /v1/manual-replication/check-units?"
            + "storageType={storageType}&storageId={storageId}&resourceId={resourceId}")
    @Headers({"x-auth-token: {token}"})
    void checkBeforeManualReplication(URI uri, @Param("token") String token, @Param("storageType") String storageType,
            @Param("storageId") String storageId, @Param("resourceId") String resourceId);
}
