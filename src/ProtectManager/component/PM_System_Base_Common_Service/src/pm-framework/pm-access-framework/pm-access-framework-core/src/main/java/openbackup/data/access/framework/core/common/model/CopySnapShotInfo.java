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

import lombok.Getter;
import lombok.Setter;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.apache.commons.lang3.StringUtils;

/**
 * 统一框架快照信息
 *
 */
@Slf4j
@Getter
@Setter
public class CopySnapShotInfo {
    private String id;

    private String name;

    private String parentName;

    /**
     * 根据snapshotId生成snapshot名称
     * snapshotId格式（filesystemId + @ + snapshotName）
     *
     * @return 快照名称
     */
    public String splitForSnapshotName() {
        log.info("start to split snapshotId:{}", id);
        if (StringUtils.isBlank(id)) {
            return StringUtils.EMPTY;
        }
        String[] names = id.split("@");
        if (names.length != 2) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "split for snapshot name error");
        }
        return names[1];
    }
}
