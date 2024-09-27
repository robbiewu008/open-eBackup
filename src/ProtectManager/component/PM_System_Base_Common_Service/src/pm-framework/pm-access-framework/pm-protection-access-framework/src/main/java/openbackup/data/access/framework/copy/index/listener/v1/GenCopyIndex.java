/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
