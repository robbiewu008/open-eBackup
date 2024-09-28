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
package openbackup.data.access.framework.copy.index.listener.v2;

import openbackup.data.access.framework.copy.index.service.impl.UnifiedCopyIndexService;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.model.CopyIndexResponse;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

/**
 * 统一框架副本索引开始监听器
 *
 */
@Component
@Slf4j
public class UnifiedCopyIndexListener {
    private final CopyRestApi copyRestApi;

    private final UnifiedCopyIndexService unifiedCopyIndexService;

    private final DeployTypeService deployTypeService;

    public UnifiedCopyIndexListener(CopyRestApi copyRestApi, UnifiedCopyIndexService unifiedCopyIndexService,
        DeployTypeService deployTypeService) {
        this.copyRestApi = copyRestApi;
        this.unifiedCopyIndexService = unifiedCopyIndexService;
        this.deployTypeService = deployTypeService;
    }

    /**
     * 监听创建索引文件的消息
     *
     * @param message 消息
     * @param acknowledgment ack
     */
    @ExterAttack
    @MessageListener(topics = TopicConstants.GEN_INDEX, groupId = "indexConsumerGroup")
    public void indexStart(String message, Acknowledgment acknowledgment) {
        // 安全一体机和主存防勒索不支持索引
        if (deployTypeService.isCyberEngine() || deployTypeService.isHyperDetectDeployType()) {
            acknowledgment.acknowledge();
            return;
        }
        CopyIndexResponse copyIndexInfo = JSONObject.toBean(message, CopyIndexResponse.class);
        String copyId = copyIndexInfo.getCopyId();
        if (StringUtils.isBlank(copyId)) {
            log.info("Copy id is empty.");
            acknowledgment.acknowledge();
            return;
        }
        Copy copy = copyRestApi.queryCopyByID(copyId, false);
        if (copy == null) {
            log.info("no query copy, do nothing. copy id is {}", copyId);
            acknowledgment.acknowledge();
            return;
        }
        log.info("start to create copy: {} index task.", copyId);
        CopyBo copyBo = new CopyBo();
        BeanUtils.copyProperties(copy, copyBo);
        unifiedCopyIndexService.createIndexTask(copyBo, copyIndexInfo.getRequestId(), copyIndexInfo.getGenIndex());
        acknowledgment.acknowledge();
    }
}
