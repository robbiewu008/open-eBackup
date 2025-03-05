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
package openbackup.data.access.framework.copy.mng.service.impl;

import com.huawei.oceanprotect.repository.tapelibrary.common.constant.MediaSetStatus;
import com.huawei.oceanprotect.repository.tapelibrary.service.MediaSetStatusChangedObserver;
import com.huawei.oceanprotect.repository.tapelibrary.service.bo.MediaSetBo;

import com.google.common.collect.ImmutableList;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyStorageUnitStatus;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * Copy Media Set Status observer
 *
 */
@Slf4j
@Component
public class CopyStatusChangedByMediaSetStatus implements MediaSetStatusChangedObserver {
    private static final ImmutableList<Integer> ALLOW_COPY_ONLINE_MEDIA_SET_STATUS = ImmutableList
            .of(MediaSetStatus.ONLINE.getValue(), MediaSetStatus.PART_ONLINE.getValue());

    @Autowired
    CopyManagerService copyManagerService;

    @Autowired
    CopyRestApi copyRestApi;

    /**
     * 存储池状态变化，修改副本状态
     *
     * @param mediaSetBoList 列表
     */
    @Override
    public void onMediaSetStatusChanged(List<MediaSetBo> mediaSetBoList) {
        log.info("media set status changed, and copies status will be changed");
        for (MediaSetBo mediaSetBo : mediaSetBoList) {
            List<String> copyIdList = copyManagerService.queryCopyIdByStorageId(
                    CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value(), mediaSetBo.getMediaSetId());
            log.info("update copies status in storage pool: {}", mediaSetBo.getMediaSetId());
            if (VerifyUtil.isEmpty(copyIdList)) {
                return;
            }
            if (ALLOW_COPY_ONLINE_MEDIA_SET_STATUS.contains(mediaSetBo.getStatus().getValue())) {
                copyManagerService.updateCopyStorageUnitStatus(copyIdList, CopyStorageUnitStatus.ONLINE.getValue());
            } else {
                copyManagerService.updateCopyStorageUnitStatus(copyIdList, CopyStorageUnitStatus.OFFLINE.getValue());
            }
        }
    }
}