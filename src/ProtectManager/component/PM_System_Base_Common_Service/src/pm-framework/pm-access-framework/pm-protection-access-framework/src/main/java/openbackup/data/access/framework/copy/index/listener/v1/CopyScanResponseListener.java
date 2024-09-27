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

import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyConstants;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.enums.CopyIndexType;
import openbackup.data.access.framework.core.common.model.CopyIndexDoneMsg;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.PageQueryRestApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.security.exterattack.ExterAttack;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Recover;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 副本扫描响应监听(VM等虚拟化应用使用)
 *
 * @author y30037959
 * @since 2023-06-01
 */
@Slf4j
@Component
public class CopyScanResponseListener {
    private final NotifyManager notifyManager;

    private final CopyRestApi copyRestApi;

    private final RedissonClient redissonClient;

    /**
     * 构造注入
     *
     * @param notifyManager notifyManager
     * @param copyRestApi copyRestApi
     * @param redissonClient redissonClient
     */
    public CopyScanResponseListener(NotifyManager notifyManager, CopyRestApi copyRestApi,
        RedissonClient redissonClient) {
        this.notifyManager = notifyManager;
        this.copyRestApi = copyRestApi;
        this.redissonClient = redissonClient;
    }

    /**
     * Consume scan response topic message
     *
     * @param consumerString copy index delete msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @Retryable(exclude = {LegoCheckedException.class}, maxAttempts = 5, backoff = @Backoff(delay = 120000))
    @KafkaListener(groupId = TopicConstants.KAFKA_CONSUMER_GROUP, topics = TopicConstants.SCAN_RESPONSE,
        autoStartup = "false")
    public void scanResponse(String consumerString, Acknowledgment acknowledgment) {
        if (StringUtils.isBlank(consumerString)) {
            log.info("Scan Response msg is invalid");
            acknowledgment.acknowledge();
            return;
        }

        JSONObject jsonObject = JSONObject.fromObject(consumerString);
        String requestId = jsonObject.getString(ContextConstants.REQUEST_ID);
        RMap<String, String> redissonClientMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String copyId = redissonClientMap.get(ContextConstants.COPY_ID);
        log.info("Consumer scan response,copyId: {},requestId: {}", copyId, requestId);
        String status = jsonObject.getString(CopyIndexConstants.STATUS);
        String errorCode = jsonObject.getString(ContextConstants.ERROR_CODE);
        boolean shouldCheckCopyResult = checkCopyStatus(errorCode, copyId, status);
        if (!shouldCheckCopyResult) {
            acknowledgment.acknowledge();
            return;
        }

        Copy copy = copyRestApi.queryCopyByID(copyId);
        if (copy == null) {
            errorCode = StringUtils.isBlank(errorCode)
                ? CopyIndexStatus.INDEX_COPY_STATUS_ERROR_LABEL.getIndexStaus()
                : errorCode;
            log.error("Not found copy, do nothing, error_code: {}", errorCode);
            copyRestApi.updateCopyIndexStatus(copyId, CopyIndexStatus.INDEX_FAIL.getIndexStaus(), errorCode);
            acknowledgment.acknowledge();
            return;
        }

        String filePath = jsonObject.getString(CopyIndexConstants.PATH);
        sendIndexMessage(filePath, copy, requestId);
        acknowledgment.acknowledge();
    }

    private boolean checkCopyStatus(String errorCode, String copyId, String status) {
        if (StringUtils.isBlank(copyId)) {
            log.error("Copy id is invalid, do nothing");
            return false;
        }

        if (!CopyIndexConstants.SUCCESS.equals(status)) {
            String errorLabel = StringUtils.isBlank(errorCode)
                ? CopyIndexStatus.INDEX_SCAN_RESPONSE_ERROR_LABEL.getIndexStaus()
                : errorCode;
            log.error("Failed to get success status of scanning,error_code: {}", errorLabel);
            copyRestApi.updateCopyIndexStatus(copyId, CopyIndexStatus.INDEX_FAIL.getIndexStaus(), errorLabel);
            return false;
        }
        return true;
    }

    /**
     * recover scan response topic message
     *
     * @param consumerString san response delete msg
     * @param acknowledgment Acknowledgment
     * @param error Exception
     */
    @Recover
    @ExterAttack
    public void recover(Exception error, String consumerString, Acknowledgment acknowledgment) {
        try {
            JSONObject jsonObject = JSONObject.fromObject(consumerString);
            String requestId = jsonObject.getString(ContextConstants.REQUEST_ID);
            RMap<String, String> redissonClientMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
            String copyId = redissonClientMap.get(ContextConstants.COPY_ID);
            log.info("Recover scan response,copyId: {},requestId: {}", copyId, requestId);
            String errorCode = CopyIndexStatus.INDEX_SCAN_RESPONSE_ERROR_LABEL.getIndexStaus();
            copyRestApi.updateCopyIndexStatus(copyId, CopyIndexStatus.UNINDEXED.getIndexStaus(), errorCode);
        } catch (Exception ex) {
            log.error("Scan response and update index status failed.", ex);
        } finally {
            acknowledgment.acknowledge();
        }
    }

    private void sendIndexMessage(String filePath, Copy copy, String requestId) {
        CopyIndexDoneMsg copyIndexDoneMsg = new CopyIndexDoneMsg();
        copyIndexDoneMsg.setCopyId(copy.getUuid());
        copyIndexDoneMsg.setPath(filePath);
        copyIndexDoneMsg.setRequestId(requestId);
        copyIndexDoneMsg.setDefaultPublishTopic(TopicConstants.GEN_INDEX_DONE);
        copyIndexDoneMsg.setResponseTopic(TopicConstants.INDEX_RESPONSE);

        RMap<String, String> redissonClientMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        String generateType = redissonClientMap.get(CopyIndexConstants.INDEX_OPERATE_TYPE);
        copyIndexDoneMsg.setIndexType(buildIndexTypeFromGenerateType(generateType));
        copyIndexDoneMsg.setPreviousIndexedGn(getPreviousIndexCopyGn(copy, generateType));
        copyRestApi.updateCopyIndexStatus(copy.getUuid(), CopyIndexStatus.INDEXING.getIndexStaus(), "");
        log.info("Update copy status as {}", CopyIndexStatus.INDEXING.getIndexStaus());
        notifyManager.send(TopicConstants.GEN_INDEX_DONE, JSONObject.fromObject(copyIndexDoneMsg).toString());
        log.info("Sent topic message[{}] successfully", TopicConstants.GEN_INDEX_DONE);
    }

    /**
     * 手动离线索引使用全量索引，自动使用增量索引。
     * indexer模块对于增量索引会自动判断是否需要做全量索引
     *
     * @param generateType 生成方式
     * @return IndexType
     */
    private String buildIndexTypeFromGenerateType(String generateType) {
        if (CopyIndexConstants.GEN_INDEX_MANUAL.equals(generateType)) {
            return CopyIndexType.FULL.getIndexType();
        }

        return CopyIndexType.INCREMENTAL.getIndexType();
    }

    private int getPreviousIndexCopyGn(Copy copy, String genIndex) {
        int curGn = 0;
        if (CopyIndexConstants.GEN_INDEX_MANUAL.equals(genIndex)) {
            return curGn;
        }

        JSONObject condition = new JSONObject().set(CopyConstants.CHAIN_ID, copy.getChainId())
            .set(CopyIndexConstants.INDEX_COLUME, CopyIndexStatus.INDEXED.getIndexStaus());
        if (StringUtils.isNotBlank(copy.getDeviceEsn())) {
            condition.set(CopyIndexConstants.DEVICE_ESN, copy.getDeviceEsn());
        }
        BasePage<Copy> copyPage = PageQueryRestApi.get(copyRestApi::queryCopies).queryAll(condition);
        if (copyPage == null) {
            log.info("no found copy info for chain_id[{}] and index status[{}]", copy.getChainId(),
                CopyIndexStatus.INDEXED.getIndexStaus());
            return curGn;
        }
        List<Copy> copyList = copyPage.getItems();
        if (copyList == null || copyList.size() == 0) {
            log.info("no found any indexed copy for chain_id[{}] by auto index", copy.getChainId());
            return curGn;
        }
        copyList.sort((o1, o2) -> o2.getGn() - o1.getGn());

        for (Copy copyTemp : copyList) {
            if (copyTemp.getGn() < copy.getGn()) {
                curGn = copyTemp.getGn();
                break;
            }
        }
        return curGn;
    }
}
