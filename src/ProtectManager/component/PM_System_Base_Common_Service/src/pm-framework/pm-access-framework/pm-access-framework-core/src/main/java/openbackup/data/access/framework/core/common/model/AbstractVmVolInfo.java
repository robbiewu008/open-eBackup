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
package openbackup.data.access.framework.core.common.model;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import org.springframework.util.CollectionUtils;

import java.util.List;

/**
 * Vm卷信息抽象类
 *
 */
@Slf4j
public abstract class AbstractVmVolInfo {
    /**
     * 获取快照信息
     *
     * @param properties 配置信息
     * @return 快照信息
     */
    protected static List<CopySnapShotInfo> obtainSnapshotInfos(JSONObject properties) {
        log.info("start parse disk info");
        List<CopySnapShotInfo> copySnapShotInfos = JSONArray.toCollection(
            properties.getJSONArray(VmIndexerCopyMetaData.SNAPSHOTS), CopySnapShotInfo.class);
        if (CollectionUtils.isEmpty(copySnapShotInfos)) {
            log.error("snapshot info empty create scan request fail");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "snapshot info empty create scan request fail");
        }
        return copySnapShotInfos;
    }
}
