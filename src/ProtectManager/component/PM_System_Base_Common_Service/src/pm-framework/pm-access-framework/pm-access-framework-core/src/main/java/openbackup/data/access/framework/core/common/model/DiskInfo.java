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

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Builder;
import lombok.Getter;
import lombok.Setter;

/**
 * 存储盘信息实体类
 *
 */
@Getter
@Setter
@Builder
public class DiskInfo {
    /**
     * 硬盘guid
     */
    @JsonProperty("GUID")
    private String guid;

    /**
     * 硬盘名
     */
    @JsonProperty("NAME")
    private String name;

    /**
     * 文件系统名
     */
    @JsonProperty("DISKDEVICENAME")
    private String fileSystemName;

    /**
     * 快照名称
     */
    @JsonProperty("DISKSNAPSHOTDEVICENAME")
    private String snapshotName;

    /**
     * 快照ID
     */
    private String snapshotId;
}
