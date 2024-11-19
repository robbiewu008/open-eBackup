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
package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 功能描述: FileSystemScrubRequest
 *
 */
@Getter
@Setter
@NoArgsConstructor
public class FileSystemScrubRequest {
    /**
     * scan range: all_metadata_and_data
     */
    private static final String DEFAULT_RANGE = "2";

    @JsonProperty("file_system_id")
    private String fsId;

    @JsonProperty("action")
    private String action;

    @JsonProperty("scan_range")
    private String range;

    /**
     * 构造方法
     *
     * @param fsId 文件系统 ID
     * @param action start: 开启; stop: 关闭
     */
    public FileSystemScrubRequest(String fsId, String action) {
        this(fsId, action, DEFAULT_RANGE);
    }

    /**
     * 构造方法
     *
     * @param fsId 文件系统 ID
     * @param action start: 开启; stop: 关闭
     * @param range 扫描范围
     */
    public FileSystemScrubRequest(String fsId, String action, String range) {
        this.fsId = fsId;
        this.action = action;
        this.range = range;
    }
}