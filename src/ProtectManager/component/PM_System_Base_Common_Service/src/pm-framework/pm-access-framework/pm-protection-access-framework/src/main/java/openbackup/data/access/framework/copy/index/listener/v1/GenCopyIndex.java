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
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

/**
 * Copy index msg listener, this lister will consume the copy index msg
 *
 * @author zwx1010134
 * @since 2021-05-28
 */
@Component
@Slf4j
public class GenCopyIndex {
    @Autowired
    private CopyRestApi copyRestApi;

    /**
     * index message recover
     *
     * @param consumerString san response delete msg
     * @param acknowledgment Acknowledgment
     * @param errorCode      errorLabel
     */
    public void genIndexRecover(String consumerString, Acknowledgment acknowledgment, String errorCode) {
        try {
            JSONObject jsonObject = JSONObject.fromObject(consumerString);
            String copyId = jsonObject.getString(ContextConstants.COPY_ID);
            log.info("Recover index response,copyId: {}", copyId);
            copyRestApi.updateCopyIndexStatus(copyId, CopyIndexStatus.UNINDEXED.getIndexStaus(), errorCode);
        } catch (Throwable ex) {
            log.error("Gen index and update index status failed.", ex);
        } finally {
            acknowledgment.acknowledge();
            log.info("Consumer copy save event success");
        }
    }
}
