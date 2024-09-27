/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.index.provider;

import openbackup.data.access.framework.core.common.constants.CopyIndexConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.model.CopyIndexDeleteMsg;
import openbackup.data.protection.access.provider.sdk.index.v1.DeleteIndexProvider;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 发送删除副本索引的消息
 *
 * @author zwx1010134
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-06-30
 */
@Slf4j
@Component
public class DeleteCopyIndexProvider implements DeleteIndexProvider {
    @Autowired
    private NotifyManager notifyManager;

    @Autowired
    private CopyRestApi copyRestApi;

    @Override
    public boolean applicable(String copyResourceSubType) {
        return ImmutableList.of(ResourceSubTypeEnum.FILESET.getType(), ResourceSubTypeEnum.VMWARE.getType(),
                ResourceSubTypeEnum.FUSION_COMPUTE.getType(), ResourceSubTypeEnum.HCS_CLOUD_HOST.getType(),
                ResourceSubTypeEnum.FUSION_ONE_COMPUTE.getType())
            .contains(copyResourceSubType);
    }

    @Override
    public void deleteIndex(String requestId, String copyId) {
        sendDeleteCopyIndexMessage(requestId, copyId);
    }

    /**
     * send delete copy index message
     *
     * @param requestId request id
     * @param copyId copy id
     */
    public void sendDeleteCopyIndexMessage(String requestId, String copyId) {
        Copy copy = null;
        try {
            copy = copyRestApi.queryCopyByID(copyId);
        } catch (LegoCheckedException exception) {
            log.error("Can't obtain copyInfo by copyId.", exception);
        }
        // 删除已经索引的文件集副本，删除时需删除索引
        if (copy != null && CopyIndexConstants.SUPPORT_INDEX_RESOURCE.contains(copy.getResourceSubType())
            && CopyIndexStatus.INDEXED.getIndexStaus().equals(copy.getIndexed())) {
            String filePath = CopyIndexConstants.COPY_INDEX_FILE_PATH + copy.getChainId()
                + CopyIndexConstants.FILE_SEPARATOR;
            String fileName = CopyIndexConstants.INDEX_FILE_PREFIX + CopyIndexConstants.UNDERSCORE_SEPARATOR
                + copy.getChainId() + CopyIndexConstants.UNDERSCORE_SEPARATOR + copy.getGn()
                + CopyIndexConstants.INDEX_FILE_SUFFIX;

            CopyIndexDeleteMsg copyIndexDeleteMsg = new CopyIndexDeleteMsg();
            copyIndexDeleteMsg.setCopyId(copy.getUuid());
            copyIndexDeleteMsg.setPath(filePath + fileName);
            copyIndexDeleteMsg.setRequestId(requestId);
            copyIndexDeleteMsg.setChainId(copy.getChainId());

            Copy[] preAndNextCopies = findPreAndNextCopy(copy);
            int prevGn = (preAndNextCopies[0] == null ? -1 : preAndNextCopies[0].getGn());
            copyIndexDeleteMsg.setPrevCopyGn(prevGn);
            int nextGn = (preAndNextCopies[1] == null ? -1 : preAndNextCopies[1].getGn());
            copyIndexDeleteMsg.setNextCopyGn(nextGn);
            copyIndexDeleteMsg.setDefaultPublishTopic(TopicConstants.DELETE_INDEX_REQUEST);
            copyIndexDeleteMsg.setResponseTopic(TopicConstants.DELETE_INDEX_RESPONSE);
            notifyManager.send(TopicConstants.DELETE_INDEX_REQUEST,
                JSONObject.fromObject(copyIndexDeleteMsg).toString());
            log.info("Sent topic: {} successfully, copyId: {}", TopicConstants.DELETE_INDEX_REQUEST, copyId);
        }
    }

    // 找copy的前后copy
    private Copy[] findPreAndNextCopy(Copy copy) {
        Copy[] res = new Copy[2];
        int page = 0;
        int size = 100;

        Map<String, Object> condition = new HashMap<>();
        condition.put("chain_id", copy.getChainId());
        condition.put("generated_by", copy.getGeneratedBy());
        condition.put("device_esn", copy.getDeviceEsn());
        List<Copy> copies;
        do {
            copies = copyRestApi.queryCopies(page++, size, condition).getItems();
            fillPreAndNextCopy(copy, res, copies);
        } while (copies.size() == size);
        return res;
    }

    private void fillPreAndNextCopy(Copy copy, Copy[] res, List<Copy> copies) {
        for (Copy elem : copies) {
            if (elem.getGn() < copy.getGn() && (res[0] == null || elem.getGn() > res[0].getGn())) {
                res[0] = elem;
            }
            if (elem.getGn() > copy.getGn() && (res[1] == null || elem.getGn() < res[1].getGn())) {
                res[1] = elem;
            }
        }
    }
}
