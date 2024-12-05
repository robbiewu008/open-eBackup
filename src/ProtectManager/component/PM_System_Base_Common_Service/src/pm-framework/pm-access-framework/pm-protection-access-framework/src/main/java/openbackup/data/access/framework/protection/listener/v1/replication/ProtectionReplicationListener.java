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
package openbackup.data.access.framework.protection.listener.v1.replication;

import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.REPLICATION_TARGET_MODE;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.USERNAME;
import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.USER_INFO;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterService;
import com.huawei.oceanprotect.functionswitch.template.service.FunctionSwitchService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants;
import com.huawei.oceanprotect.sla.common.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.ReplicationMode;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationProvider;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.Repository;
import openbackup.data.protection.access.provider.sdk.replication.ReplicationProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.RedisConstants;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.auth.UserResponse;
import openbackup.system.base.sdk.auth.model.response.UserPageListResponse;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.ApplicationContextService;

import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * Protection Replication Listener
 *
 */
@Component
@Slf4j
public class ProtectionReplicationListener {
    /**
     * JOB_STATUS_LOG
     */
    private static final String JOB_STATUS_LOG = "job_status_{payload?.job_status|status}_label";

    private static final String PROJECT_ID = "project_id";

    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private UnifiedReplicationProvider unifiedReplicationProvider;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ClusterInternalApi clusterInternalApi;

    @Autowired
    private ApplicationContextService applicationContextService;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private ResourceRestApi resourceRestApi;

    @Autowired
    private UserService userService;

    @Autowired
    private FunctionSwitchService functionSwitchService;

    @Autowired
    private UserQuotaManager userQuotaManager;

    @Autowired
    private JobService jobService;

    @Autowired
    private ClusterQueryService clusterQueryService;

    @Autowired
    @Qualifier("defaultClusterServiceImpl")
    private ClusterService clusterService;

    /**
     * copy replicate
     *
     * @param payload kafka message payload
     * @param acknowledgment ack
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.REPLICATION_TOPIC, failures = TopicConstants.REPLICATION_COMPLETE_TOPIC,
        containerFactory = "retryFactory", log = {"job_log_copy_replication_exec_label", JOB_STATUS_LOG})
    public void replicate(String payload, Acknowledgment acknowledgment) {
        JSONObject data = JSONObject.fromObject(payload);
        String requestId = data.getString("request_id");

        checkJobFinished(requestId);

        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String jsonStr = context.get("protected_object");
        ProtectedObject protectedObject = JSONObject.fromObject(jsonStr).toBean(ProtectedObject.class);
        buildResourceExtendInfo(protectedObject.getResourceId(), context);
        PolicyBo policy = data.getBean("policy", PolicyBo.class);
        Repository repository = getRepository(policy, requestId);
        TargetClusterVo targetCluster = getTargetCluster(policy, requestId, repository.getUuid());

        BackupObject backupObject = buildBackupObject(requestId, context, protectedObject, repository);
        context.put("target_cluster", JSONObject.fromObject(targetCluster).toString());
        List<String> sameChainCopies = Collections.emptyList();
        if (data.containsKey("same_chain_copies")) {
            sameChainCopies = data.getJSONArray("same_chain_copies").toBean(String.class);
        }
        RMap<String, String> map = redissonClient.getMap(
            RedisConstants.TARGET_CLUSTER_RELATED_TASKS + targetCluster.getClusterId(), StringCodec.INSTANCE);
        if (StringUtils.isNotBlank(protectedObject.getResourceId())) {
            // 反向复制没有保护对象
            map.put(backupObject.getTaskId(), protectedObject.getResourceId());
        }
        log.info("start to replicate.request id is: {}", requestId);
        String resourceSubType = protectedObject.getSubType();
        ReplicationProvider replicationProvider = providerManager.findProviderOrDefault(ReplicationProvider.class,
            resourceSubType, unifiedReplicationProvider);
        ResourceEntity resourceEntity = buildResourceEntity(data);

        // hcs 复制策略字段 只有hcs线上环境由hcs用户创建策略时会存在
        JsonNode projectId = policy.getExtParameters().get(PROJECT_ID);
        String targetUserId = VerifyUtil.isEmpty(projectId) ? Strings.EMPTY : projectId.asText();
        String resourceUserId = userQuotaManager.getUserId(resourceEntity.getUserId(), resourceEntity.getRootUuid());

        // 所有用户进行复制限额
        replicationPreCheck(resourceUserId, targetCluster, targetUserId, policy);
        resetTargetResourceUserId(resourceEntity, targetUserId, resourceUserId);
        ReplicateContext replicateContext = applicationContextService.autowired(
            ReplicateContext.builder().context(context).sameChainCopies(sameChainCopies).requestId(requestId).build(),
            Arrays.asList(backupObject, resourceEntity, policy, targetCluster));
        try {
            replicationProvider.replicate(replicateContext);
        } catch (RuntimeException e) {
            log.error("Execute replicate failed. taskId:{}, resourceId:{}. errorMsg: ", backupObject.getTaskId(),
                protectedObject.getResourceId(), ExceptionUtil.getErrorMessage(e));
            // 执行失败，需要清除redis中写入的目标集群关联信息
            map.remove(backupObject.getTaskId());
            throw e;
        }
    }

    private void resetTargetResourceUserId(ResourceEntity resourceEntity, String targetUserId, String userId) {
        resourceEntity.setUserId(!VerifyUtil.isEmpty(targetUserId) ? targetUserId : userId);
    }

    private Repository getRepository(PolicyBo policy, String requestId) {
        Repository repository = new Repository();
        if (isExtra(policy)) {
            repository = buildRepository(policy);
        } else {
            if (policy.getExtParameters().has(ExtParamsConstants.EXTERNAL_STORAGE_ID) && StringUtils.isNotEmpty(
                policy.getExtParameters().get(ExtParamsConstants.EXTERNAL_STORAGE_ID).asText())) {
                String storageId = policy.getExtParameters().get(ExtParamsConstants.EXTERNAL_STORAGE_ID).textValue();
                repository = new Repository();
                repository.setUuid(storageId);
            }
        }
        return repository;
    }

    private TargetClusterVo getTargetCluster(PolicyBo policy, String requestId, String clusterId) {
        TargetClusterVo targetCluster;
        if (isExtra(policy)) {
            log.info("before query target cluster.request id is: {}", requestId);
            targetCluster = queryTargetCluster(clusterId);
            log.info("Replicate data to target cluster: {}", targetCluster.getClusterId());
        } else {
            targetCluster = clusterQueryService.getMemberClusterDetail().getTargetClusterVo();
        }
        return targetCluster;
    }

    private void checkJobFinished(String requestId) {
        // 判断任务是否已完成，kafka去重
        JobBo job = jobService.queryJob(requestId);
        if (JobStatusEnum.get(job.getStatus()).finishedStatus()) {
            log.info("Job was already finished.requestId:{}", requestId);
            return;
        }
    }

    private boolean isExtra(PolicyBo policyBo) {
        int replicationMode = policyBo.getIntegerFormExtParameters(REPLICATION_TARGET_MODE,
            ReplicationMode.EXTRA.getValue());
        return SlaConstants.EXTRA_REPLICATION_MODE.contains(replicationMode);
    }

    private void replicationPreCheck(String resourceUserId, TargetClusterVo targetCluster, String targetUserId,
        PolicyBo policyBo) {
        log.info("Start to replicationPreCheck!resourceUserId:{},clusterId:{}, targetUserId:{}", resourceUserId,
            targetCluster.getClusterId(), targetCluster);

        if (isExtra(policyBo)) {
            checkExtraRepQuota(policyBo, targetUserId, targetCluster);
        } else {
            checkInnerRepQuota(resourceUserId, targetCluster);
        }
    }

    private void checkExtraRepQuota(PolicyBo policyBo, String targetUserId, TargetClusterVo targetCluster) {
        String dpUserName = getDpUserName(policyBo.getExtParameters());
        // 1.5升级1.6默认是远端设备管理员用户信息
        if (StringUtils.isEmpty(dpUserName) || dpUserName.equals(targetCluster.getUsername())) {
            return;
        }
        UserPageListResponse<UserResponse> allDPUser = userService.getAllDPUser(
            Integer.parseInt(targetCluster.getClusterId()));

        if (allDPUser.getUserList() == null) {
            throw new LegoCheckedException(CommonErrorCode.USER_OR_STORAGE_UNIT_NOT_EXIST,
                "Can not get target cluster user");
        }

        UserResponse targetUser = allDPUser.getUserList()
            .stream()
            .filter(userResponse -> userResponse.getUserName().equals(dpUserName)
                || userResponse.getUserId().equals(targetUserId))
            .findAny()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.USER_OR_STORAGE_UNIT_NOT_EXIST,
                "Target user not exist"));
        if (UserTypeEnum.HCS.getValue().equals(targetUser.getUserType()) && !VerifyUtil.isEmpty(targetUserId)) {
            userQuotaManager.checkHcsUserReplicationQuota(Integer.valueOf(targetCluster.getClusterId()), targetUserId);
            return;
        }
            checkBackUserQuota(targetCluster, targetUser);
    }

    private String getDpUserName(JsonNode extParameters) {
        if (!extParameters.has(USER_INFO)) {
            return Strings.EMPTY;
        }
        String dpUserName = null;
        if (extParameters != null && extParameters.has(USER_INFO) && extParameters.get(USER_INFO).has(USERNAME)) {
            dpUserName = extParameters.get(USER_INFO).get(USERNAME).textValue();
        }

        if (VerifyUtil.isEmpty(dpUserName)) {
            throw new LegoCheckedException(CommonErrorCode.USER_OR_STORAGE_UNIT_NOT_EXIST, "Target user not exist");
        }
        return dpUserName;
    }

    private void checkInnerRepQuota(String resourceUserId, TargetClusterVo targetCluster) {
        UserInnerResponse userInnerResponse = null;
        if (StringUtils.isEmpty(resourceUserId)) {
            try {
                userInnerResponse = userService.getUserInfoByUserId(resourceUserId);
            } catch (LegoCheckedException e) {
                log.warn("The user:{} not exist!Do not start replication pre check!", resourceUserId);
            }
        }
        if (userInnerResponse == null) {
            return;
        }
        checkBackUserQuota(targetCluster, userInnerResponse);
    }

    private void checkBackUserQuota(TargetClusterVo targetCluster, UserInnerResponse userInnerResponse) {
        log.info("Start to check backup quota in target cluster!userId:{}", userInnerResponse.getUserId());
        // 校验目标端备份额度
        userQuotaManager.checkUserBackupQuotaInTargetWhenReplication(Integer.valueOf(targetCluster.getClusterId()),
            userInnerResponse.getUserId());
    }

    private ResourceEntity buildResourceEntity(JSONObject data) {
        ResourceEntity resource = data.getBean("resource_obj", ResourceEntity.class);
        // 级联复制和反向复制资源对象从kafka消息中取出
        return resource != null
            ? resource
            : resourceRestApi.queryResourceById(data.getString("resource_id"), ResourceEntity.class);
    }

    private void buildResourceExtendInfo(String resourceId, RMap<String, String> context) {
        Optional<ProtectedResource> resource = resourceService.getResourceById(resourceId);
        resource.ifPresent(protectedResource -> {
            // 将资源的扩展信息更新到资源中，并更新Redis缓存
            JSONObject jsonObject = JSONObject.fromObject(context.get(ContextConstants.RESOURCE))
                .set("extendInfo", protectedResource.getExtendInfo());
            context.put(ContextConstants.RESOURCE, jsonObject.toString());
        });
    }

    private TargetClusterVo queryTargetCluster(String clusterId) {
        if (VerifyUtil.isEmpty(clusterId)) {
            log.error("Not found the target cluster, cluster id is empty.");
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "Not found the target cluster, cluster id is empty");
        }
        // 尝试获取远端设备token,如果获取失败提示原因
        getTargetToken(clusterId);
        TargetClusterVo targetClusterVo = null;
        for (int i = 0; i < IsmNumberConstant.THREE; i++) {
            try {
                targetClusterVo = clusterInternalApi.queryTargetClusterDetailsByClusterId(Integer.parseInt(clusterId));
            } catch (Exception e) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                    "Not found the target cluster");
            }
            if (!VerifyUtil.isEmpty(targetClusterVo.getMgrIpList())) {
                break;
            }
        }

        if (!VerifyUtil.isEmpty(targetClusterVo.getMgrIpList()) && !VerifyUtil.isEmpty(targetClusterVo)) {
            return targetClusterVo;
        } else {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED, "cluster mgrIpList is null");
        }
    }

    private void getTargetToken(String clusterId) {
        try {
            clusterService.getTargetClusterToken(Integer.valueOf(clusterId));
        } catch (LegoCheckedException exception) {
            // 目标集群用户密码过期，提示精确原因及错误信息
            if (exception.getErrorCode() == CommonErrorCode.EXTERNAL_CLUSTER_PASSWORD_FIRST_MODIFY_NOTICE) {
                throw new LegoCheckedException(CommonErrorCode.REPLICATION_CLUSTER_PASSWORD_EXPIRE,
                    exception.getMessage());
            }
        }
    }

    /**
     * BackupObject
     *
     * @param requestId requestId
     * @param redisClientMap redisClientMap
     * @param protectedObject protectedObject
     * @param repository repository
     * @return BackupObject
     */
    public BackupObject buildBackupObject(String requestId, RMap<String, String> redisClientMap,
        ProtectedObject protectedObject, Repository repository) {
        BackupObject backupObject = new BackupObject();
        backupObject.setRequestId(requestId);
        backupObject.setProtectedObject(protectedObject);
        backupObject.setTaskId(redisClientMap.get("job_id"));
        backupObject.setRepository(repository);
        return backupObject;
    }

    /**
     * build repository
     *
     * @param policy policy
     * @return Repository
     */
    public Repository buildRepository(PolicyBo policy) {
        ReplicationExtParam param = JSONObject.fromObject(policy.getExtParameters()).toBean(ReplicationExtParam.class);
        Repository repository = new Repository();
        String uuid = param.getExternalSystemId();
        repository.setUuid(uuid);
        return repository;
    }
}
