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
package openbackup.data.access.framework.copy.browser.listener;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.VmBrowserMountStatus;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.annotation.KafkaListener;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.retry.annotation.Backoff;
import org.springframework.retry.annotation.Retryable;
import org.springframework.stereotype.Component;

/**
 * 虚拟化资源细粒度浏览副本挂载状态监听
 *
 */
@Slf4j
@Component
public class CopyBrowserMountListener {
    @Autowired
    private CopyRestApi copyRestApi;

    /**
     * Consume copy index topic message
     *
     * @param consumerString copy index msg
     * @param acknowledgment Acknowledgment
     */
    @ExterAttack
    @Retryable(exclude = {LegoCheckedException.class}, maxAttempts = 5, backoff = @Backoff(delay = 120000))
    @KafkaListener(groupId = TopicConstants.KAFKA_CONSUMER_GROUP, topics = TopicConstants.VM_BROWSE_MOUNT_RESPONSE,
        autoStartup = "false")
    public void copyBrowseMountResponse(String consumerString, Acknowledgment acknowledgment) {
        if (StringUtils.isBlank(consumerString)) {
            log.info("vmBrowseMountResponse msg is invalid");
            return;
        }
        JSONObject jsonObject = JSONObject.fromObject(consumerString);
        String copyId = jsonObject.getString(ContextConstants.COPY_ID);
        log.info("Start consumer vm mount response copyId: {} ", copyId);
        if (StringUtils.isBlank(copyId)) {
            log.info("Copy id is invalid, do nothing");
            acknowledgment.acknowledge();
            return;
        }
        // 更新状态
        String status = jsonObject.getString(CopyIndexConstants.STATUS);
        String browseMountStatus;
        if (CopyIndexConstants.ABORT.equals(status)) {
            // 自动解挂载
            browseMountStatus = VmBrowserMountStatus.UMOUNT.getBrowserMountStatus();
        } else if (CopyIndexConstants.SUCCESS.equals(status)) {
            // 挂载成功
            browseMountStatus = VmBrowserMountStatus.MOUNTED.getBrowserMountStatus();
        } else {
            // 挂载失败
            browseMountStatus = VmBrowserMountStatus.MOUNT_FAIL.getBrowserMountStatus();
        }
        copyRestApi.updateCopyBrowseMountStatus(copyId, browseMountStatus);
        log.info("update copy status as {} of copy {}", browseMountStatus, copyId);
        acknowledgment.acknowledge();
    }
}
