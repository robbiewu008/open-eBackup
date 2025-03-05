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
package openbackup.data.access.framework.protection.handler.v1.replication;

import static com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants.EXTERNAL_SYSTEM_ID;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.MultiClusterAuthenticationService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.sla.sdk.api.ReplicationSlaUserService;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.model.RepliacatWormContext;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.DmeJobStatusEnum;
import openbackup.data.access.framework.core.dao.CopyMapper;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.access.framework.protection.handler.v2.UnifiedTaskCompleteHandler;
import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationCopyProcessor;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.bean.CopiesEntity;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.RedisConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.JobSpeedConverter;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.cluster.model.TokenResponse;
import openbackup.system.base.sdk.copy.model.CopyWormStatus;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.ScheduleBo;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.ProviderRegistry;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RList;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicStampedReference;
import java.util.stream.Collectors;

/**
 * Replication Task Complete Handler
 *
 */
@Component
@Slf4j
public class ReplicationTaskCompleteHandler implements TaskCompleteHandler {
    /**
     * 缓存需要同步worm状态的copy
     */
    public static final String WORM_SETTINGS = "WormSettings";

    /**
     * 复制policy触发类型，备份完立刻复制
     */
    public static final Integer AFTER_BACKUP_COMPLETE = 2;

    /**
     * 复制policy中的用户信息
     */
    public static final String USER_INFO = "user_info";

    /**
     * 复制policy中的用户密码
     */
    public static final String USER_PASSWORD = "password";

    /**
     * 复制policy中的用户名
     */
    public static final String USERNAME = "username";

    /**
     * 复制policy中的用户类型
     */
    public static final String USERTYPE = "userType";

    @Autowired
    private ReplicationSlaUserService replicationSlaUserService;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private ProviderRegistry registry;

    @Autowired
    private MessageTemplate<?> messageTemplate;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private UnifiedReplicationCopyProcessor unifiedReplicationCopyProcessor;

    @Autowired
    @Qualifier("defaultClusterServiceImpl")
    private ClusterService clusterService;

    @Autowired
    private EncryptorService encryptorService;

    @Autowired
    private SlaQueryService slaQueryService;
    @Autowired
    private CopyMapper copyMapper;
    @Autowired
    private MultiClusterAuthenticationService multiClusterAuthenticationService;

    /**
     * task success handler
     *
     * @param taskCompleteMessage task complete message
     */
    @ExterAttack
    @Override
    public void onTaskCompleteSuccess(TaskCompleteMessageBo taskCompleteMessage) {
        String requestId = taskCompleteMessage.getJobRequestId();
        RMap<String, String> context = redissonClient.getMap(requestId, StringCodec.INSTANCE);

        cleanTargetClusterRelatedTaskInfo(context);
        String jsonStr = context.get("protected_object");
        ProtectedObject protectedObject = JSONObject.fromObject(jsonStr).toBean(ProtectedObject.class);
        ReplicationCopyProcessor processor =
                registry.findProviderOrDefault(
                        ReplicationCopyProcessor.class, protectedObject.getSubType(), unifiedReplicationCopyProcessor);
        boolean isComplete = true;
        if (processor != null) {
            try {
                AtomicStampedReference<Boolean> stampedReference = processor.process(taskCompleteMessage);
                int count = stampedReference.getStamp();
                isComplete = stampedReference.getReference();
                recordeReplicatedCopyNumber(taskCompleteMessage, count);
            } catch (LegoCheckedException e) {
                log.error("process replication copy failed. request id: {}", requestId, e);
                taskCompleteMessage.setJobStatus(DmeJobStatusEnum.FAIL.getTypeName());
            }
        }
        if (isComplete) {
            JSONObject message = JSONObject.fromObject(taskCompleteMessage).set("request_id", requestId);
            message.remove("job_status");
            messageTemplate.send(TopicConstants.REPLICATION_COMPLETE, message);

            wormRepCopyDeal(taskCompleteMessage, requestId, context, protectedObject);
        }
    }

    private void wormRepCopyDeal(TaskCompleteMessageBo taskCompleteMessage, String requestId,
        RMap<String, String> context, ProtectedObject protectedObject) {
        // 是否需要同步 ，policy是复制的
        String policy = context.get("policy");
        if (StringUtils.isEmpty(policy)) {
            log.warn("Invalid policy: {}", policy);
            return;
        }
        String token = null;
        try {
            PolicyBo policyBo = JSONObject.fromObject(policy).toBean(PolicyBo.class);
            String resourceId = protectedObject.getResourceId();
            // 本次复制是备份完立刻复制；
            ScheduleBo schedule = policyBo.getSchedule();
            if (schedule != null && !AFTER_BACKUP_COMPLETE.equals(schedule.getTrigger())) {
                // 只有备份完立刻复制才需要同步worm
                log.info(
                    "This replication is not immediately after the backup, resourceId: {}, policy: {}",
                    resourceId, policy);
                return;
            }
            Map<String, String> repOrignMap = new HashMap<>();
            // 检测副本是否是worm或worm检测状态
            // 主端有、从端没有；副本状态为seting或worm，则需要同步；副本放入缓存中，定时任务拉取，等待副本worm完成；同步worm信息；
            List<String> copies = new ArrayList<>();
            PolicyDto policyDto = slaQueryService.queryPolicyById(policyBo.getUuid());
            JsonNode extParameters = policyDto.getExtParameters();
            int clusterId = extParameters.get(EXTERNAL_SYSTEM_ID).asInt();
            JsonNode userInfo = extParameters.get(USER_INFO);
            TargetCluster targetCluster = clusterService.getClusterByClusterId(clusterId);
            token = getToken(getPassword(policyDto, userInfo), userInfo, targetCluster);
            // 资源有worm，查询从端是否有worm，如果有，则返回；v1/anti-ransomware/{resource_id}/is-exist-worm-policy
            boolean existWormRemote = multiClusterAuthenticationService.isExistWormPolicyByResourceId(targetCluster,
                token, resourceId);
            if (existWormRemote) {
                // 对端有worm策略，不用同步worm状态
                log.info("The remote has worm policy, resourceId: {}, clusterId: {}, policy: {}", resourceId, clusterId,
                    policy);
                return;
            }

            injectCopyIds(taskCompleteMessage, repOrignMap, copies, targetCluster, token);
            List<CopiesEntity> copiesEntities = getCopiesEntities(requestId, resourceId, clusterId, copies);
            if (CollectionUtils.isEmpty(copiesEntities)) {
                return;
            }
            addNeedNotifyRepCopy(policyBo, resourceId, clusterId, repOrignMap, copiesEntities);
        } catch (LegoCheckedException e) {
            log.error("Error processing policy: {}", policy, ExceptionUtil.getErrorMessage(e));
        } finally {
            StringUtil.clean(token);
        }
    }

    private void addNeedNotifyRepCopy(PolicyBo policyBo, String resourceId, int clusterId,
        Map<String, String> repOrignMap, List<CopiesEntity> copiesEntities) {
        List<RepliacatWormContext> wormContexts = copiesEntities.stream().map(copy -> {
            RepliacatWormContext repliacatWormContext = new RepliacatWormContext();
            repliacatWormContext.setCopyId(copy.getUuid());
            repliacatWormContext.setRepId(repOrignMap.get(copy.getUuid()));
            repliacatWormContext.setWormStatus(copy.getWormStatus());
            repliacatWormContext.setRepPolicyId(policyBo.getUuid());
            repliacatWormContext.setResourceId(resourceId);
            repliacatWormContext.setClusterId(clusterId);
            return repliacatWormContext;
        }).collect(Collectors.toList());
        RList<RepliacatWormContext> wormSettings = redissonClient.getList(WORM_SETTINGS);
        wormSettings.addAll(wormContexts);
    }

    private String getPassword(PolicyDto policyDto, JsonNode userInfo) {
        return userInfo.has(USER_PASSWORD)
            ? userInfo.get(USER_PASSWORD).asText()
            : replicationSlaUserService.queryRepUserByPolicyId(policyDto.getUuid())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "policy not exits"))
                .getPassword();
    }

    private String getToken(String password, JsonNode userInfo, TargetCluster targetCluster) {
        String username = userInfo.get(USERNAME).asText();
        String userType = userInfo.get(USERTYPE).asText();
        targetCluster.setUsername(username);
        targetCluster.setPassword(encryptorService.encrypt(password));
        targetCluster.setUserType(userType);
        // 获取token，使用sla中的账户，而不是集群的
        TokenResponse tokenResponse = multiClusterAuthenticationService.getTokenByTargetClusterUsername(targetCluster);
        return tokenResponse.getToken();
    }

    private List<CopiesEntity> getCopiesEntities(String requestId, String resourceId, int clusterId,
        List<String> copies) {
        // 创建 LambdaQueryWrapper 对象
        LambdaQueryWrapper<CopiesEntity> lambdaQueryWrapper = new LambdaQueryWrapper<>();
        // 使用 in 方法指定查询条件
        lambdaQueryWrapper.in(CopiesEntity::getUuid, copies);
        lambdaQueryWrapper.in(CopiesEntity::getWormStatus,
            Arrays.asList(CopyWormStatus.SETTING.getStatus(), CopyWormStatus.SET_SUCCESS.getStatus()));

        List<CopiesEntity> copiesEntities = copyMapper.selectList(lambdaQueryWrapper);
        if (copiesEntities.isEmpty()) {
            log.info(
                "The replication copies is not worm or worm checking, requestId: {}, resourceId: {}, clusterId: {}",
                requestId, resourceId, clusterId);
            return new ArrayList<>();
        }
        return copiesEntities;
    }

    private void injectCopyIds(TaskCompleteMessageBo taskCompleteMessage, Map<String, String> repOrignMap,
        List<String> copies, TargetCluster targetCluster, String token) {
        // 本次复制完成的副本ID
        JSONArray backupCopyList = JSONArray.fromObject(
            taskCompleteMessage.getExtendsInfo().get("backup_copy_list"));
        if (backupCopyList != null && !backupCopyList.isEmpty()) {
            // 遍历 JSONArray 中的字符串元素
            for (int i = 0; i < backupCopyList.size(); i++) {
                String copyId = backupCopyList.getString(i);
                String originId = multiClusterAuthenticationService.queryBackUpIdByCopyId(targetCluster, token, copyId);
                repOrignMap.put(originId, copyId);
                copies.add(originId);
            }
        }
    }

    /**
     * task fail handler
     *
     * @param taskCompleteMessage task complete message
     */
    @Override
    public void onTaskCompleteFailed(TaskCompleteMessageBo taskCompleteMessage) {
        // 失败的逻辑已做处理，本次仅做接口适配
        onTaskCompleteSuccess(taskCompleteMessage);
    }

    private void recordeReplicatedCopyNumber(TaskCompleteMessageBo taskCompleteMessage, int count) {
        if (count < 0) {
            return;
        }
        String requestId = taskCompleteMessage.getJobRequestId();
        RMap<String, String> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String jobId = map.get("job_id");
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo("job_log_copy_replication_replicated_label");
        jobLogBo.setLogInfoParam(Collections.singletonList(String.valueOf(count)));
        jobLogBo.setLevel(JobLogLevelEnum.INFO.getValue());
        UpdateJobRequest request = new UpdateJobRequest();
        String jobStatus = map.get("job_status");
        log.info("update replication job status: {}", jobStatus);
        JobStatusEnum status = JobStatusEnum.get(jobStatus);
        request.setStatus(status);
        request.setJobLogs(Collections.singletonList(jobLogBo));
        if (taskCompleteMessage.getExtendsInfo() != null) {
            request.setExtendStr(JSONObject.writeValueAsString(taskCompleteMessage.getExtendsInfo()));
        }
        Long speed = taskCompleteMessage.getSpeed();
        if (speed != null) {
            String jobSpeed = JobSpeedConverter.convertJobSpeed(String.valueOf(speed));
            request.setSpeed(jobSpeed);
        }
        log.info("Get replication speed, speed: {}, job_id: {}", request.getSpeed(), jobId);
        jobCenterRestApi.updateJob(jobId, request);
    }

    /**
     * clean Target Cluster Related Task Info
     *
     * @param context context
     */
    public void cleanTargetClusterRelatedTaskInfo(RMap<String, String> context) {
        TargetClusterVo targetCluster =
                JSONObject.fromObject(context.get("target_cluster")).toBean(TargetClusterVo.class);
        RMap<String, String> map =
                redissonClient.getMap(
                        RedisConstants.TARGET_CLUSTER_RELATED_TASKS + targetCluster.getClusterId(),
                        StringCodec.INSTANCE);
        String jobId = context.get("job_id");
        map.remove(jobId);
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return JobTypeEnum.COPY_REPLICATION.getValue().equals(object)
            || (JobTypeEnum.COPY_REPLICATION.getValue() + "-" + UnifiedTaskCompleteHandler.V2).equals(object);
    }
}
