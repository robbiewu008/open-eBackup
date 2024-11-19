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
package openbackup.data.access.framework.copy.index.listener.v1;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.constant.CopyResourcePropertiesConstant;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.model.CopyIndexResponse;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JSONObjectCovert;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Recover;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * Copy index msg listener, this lister will consume the copy index msg
 *
 */
@Component
@Slf4j
public class CopyIndexListener extends GenCopyIndex {
    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private AsyncGenIndex asyncGenIndex;

    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * 监听爱数生成索引文件的消息
     *
     * @param consumerString 消息
     * @param acknowledgment ack
     */
    @ExterAttack
    @Retryable(exclude = {LegoCheckedException.class}, maxAttempts = 5, backoff = @Backoff(delay = 120000))
    @KafkaListener(groupId = "consumerGroup", topics = TopicConstants.GEN_INDEX, autoStartup = "false")
    public void genIndexer(String consumerString, Acknowledgment acknowledgment) {
        // 安全一体机和主存防勒索不支持索引
        if (deployTypeService.isCyberEngine() || deployTypeService.isHyperDetectDeployType()) {
            acknowledgment.acknowledge();
            return;
        }
        CopyIndexResponse copyIndexResponse = JSONObject.toBean(consumerString, CopyIndexResponse.class);
        String copyId = copyIndexResponse.getCopyId();
        log.info("Start consumer gen index request copyId: {} ", copyId);
        if (StringUtils.isBlank(copyId)) {
            log.info("Copy id is invalid, do nothing");
            acknowledgment.acknowledge();
            return;
        }

        if (!checkDeePodExist()) {
            log.info("Module DEE is not normal, not generate the index for copyId: {}.", copyId);
            acknowledgment.acknowledge();
            return;
        }

        Copy copy = copyRestApi.queryCopyByID(copyId, false);
        if (copy == null) {
            log.info("no found copy, do nothing");
            acknowledgment.acknowledge();
            return;
        }

        if (!CopyIndexConstants.SUPPORT_INDEX_RESOURCE.contains(copy.getResourceSubType())) {
            log.info("resource type[{}] not support copy index(CopyIndexListener), do nothing. copy id is {}",
                copy.getResourceSubType(), copyId);
            acknowledgment.acknowledge();
            return;
        }

        if (CopyIndexConstants.NOT_SUPPORT_INDEX_GENERATED_BY.contains(copy.getGeneratedBy())) {
            log.info("Index not support {}, do nothing, copy id is {}", copy.getGeneratedBy(), copyId);
            acknowledgment.acknowledge();
            return;
        }

        String genIndex = copyIndexResponse.getGenIndex();
        if (CopyIndexConstants.NEED_TO_CHECK_SLA_CONFIG_INDEX_RESOURCE.contains(copy.getResourceSubType())
            && !CopyIndexConstants.GEN_INDEX_MANUAL.equals(genIndex) && !existFlrSwitch(copy)) {
            log.info("sla not open flr switch for {}, do nothing, copy id is {}", copy.getResourceSubType(), copyId);
            acknowledgment.acknowledge();
            return;
        }

        String requestId = copyIndexResponse.getRequestId();
        if (CopyIndexConstants.NEED_TO_SAVE_CONTEXT_INDEX_RESOURCE.contains(copy.getResourceSubType())) {
            updateRedisCache(copy, genIndex, requestId);
        }
        log.info("After consumer gen index request requestId: {} copyId: {} ", requestId, copyId);
        generateIndex(acknowledgment, copy, requestId);
    }

    private void generateIndex(Acknowledgment acknowledgment, Copy copy, String requestId) {
        CopyBo copyBo = new CopyBo();
        BeanUtils.copyProperties(copy, copyBo);
        asyncGenIndex.generateIndexFileAsync(requestId, copyBo);
        acknowledgment.acknowledge();
    }

    @ExterAttack
    private void updateRedisCache(Copy copy, String genIndex, String requestId) {
        RMap<String, String> redissonClientMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        redissonClientMap.put(CopyIndexConstants.INDEX_OPERATE_TYPE, genIndex);
        redissonClientMap.put(ContextConstants.COPY_ID, copy.getUuid());
    }

    /**
     * recover scan response topic message
     *
     * @param consumerString san response delete msg
     * @param acknowledgment Acknowledgment
     * @param exception Exception
     */
    @Recover
    public void recover(Exception exception, String consumerString, Acknowledgment acknowledgment) {
        String errorCode = CopyIndexStatus.INDEX_COPY_STATUS_ERROR_LABEL.getIndexStaus();
        genIndexRecover(consumerString, acknowledgment, errorCode);
    }

    private boolean existFlrSwitch(Copy copy) {
        String slaProperties = copy.getSlaProperties();
        SlaBo slaBo = JSONObject.toBean(slaProperties, SlaBo.class);
        List<PolicyBo> policyBoList = slaBo.getPolicyList();
        if (policyBoList == null || policyBoList.size() == 0) {
            log.info("Sla policy is null, do nothing");
            return false;
        }
        // cloudbackup 索引标识从sla的高级参数中取，其他部署形态从保护对象的高级参数中取
        final JSONObject resourcePropertiesJsonObject = JSONObjectCovert.covertLowerUnderscoreKeyToLowerCamel(
                JSONObject.fromObject(copy.getResourceProperties()));
        final ProtectedObject protectedObject = resourcePropertiesJsonObject.toBean(ProtectedObject.class);
        Map<String, Object> extParameters = protectedObject.getExtParameters();
        if (extParameters != null) {
            log.info("Check resource: {} is had auto index tag.", protectedObject.getResourceId());
            Boolean backupResAutoIndex =
                MapUtils.getBoolean(extParameters, CopyResourcePropertiesConstant.BACKUP_RES_AUTO_INDEX, null);
            if (!VerifyUtil.isEmpty(backupResAutoIndex)) {
                log.info("Resource: {} backup auto index tag is: {}", protectedObject.getResourceId(),
                        backupResAutoIndex);
                return backupResAutoIndex;
            }
        }
        if (!deployTypeService.isCloudBackup()) {
            return false;
        }
        // 从备份策略中获取细粒度恢复字段值
        JsonNode fineGrainedRestorePara = null;
        for (PolicyBo policy : policyBoList) {
            if (CopyIndexConstants.BACK_UP.equals(policy.getType())) {
                fineGrainedRestorePara = policy.getExtParameters().get(CopyIndexConstants.FINE_GRAINED_RESTORE);
                break;
            }
        }
        if (fineGrainedRestorePara == null) {
            log.info("fine_grained_restore is null, do nothing");
            return false;
        }
        String fineGrainedRestore = String.valueOf(fineGrainedRestorePara);
        if (!CopyIndexConstants.FLR_SWITCH_ON.equals(fineGrainedRestore)) {
            log.info("fine_grained_restore is false, do nothing");
            return false;
        }
        return true;
    }

    private boolean checkDeePodExist() {
        InfraResponseWithError<List<NodePodInfo>> podInfos =
                infrastructureRestApi.getInfraPodInfo("protectengine");
        if (podInfos == null || podInfos.getData() == null || podInfos.getData().size() == 0) {
            log.info("DEE Pod Module not exist, not generate the index.");
            return false;
        }

        List<NodePodInfo> nodePodList = podInfos.getData();
        for (NodePodInfo nodePodInfo : nodePodList) {
            if (!nodePodInfo.getPodName().contains("protectengine")) {
                continue;
            }

            if ("Running".equals(nodePodInfo.getPodStatus())) {
                log.info("Module DEE is running and generate the index.");
                return true;
            }
        }
        log.info("DEE Pod Module not exist, not generate the index.");
        return false;
    }
}
