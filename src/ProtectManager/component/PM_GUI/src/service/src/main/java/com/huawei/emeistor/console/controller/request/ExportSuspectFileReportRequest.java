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
package com.huawei.emeistor.console.controller.request;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.Max;
import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * 导出可疑文件列表请求体
 *
 **/
@Getter
@Setter
public class ExportSuspectFileReportRequest {
    /**
     * 设备ID
     */
    @NotNull
    @Max(128)
    private String deviceId;

    /**
     * 文件系统名称
     */
    @NotNull
    @Max(1024)
    private String fileSystemName;

    /**
     * 租戶ID
     */
    @NotNull
    @Max(128)
    private String vstoreId;

    /**
     * 所属快照名
     */
    @NotNull
    @Max(1024)
    private String snapShotName;

    /**
     * 语言格式
     */
    @NotNull
    @Pattern(regexp = "en|zh", message = "lang must be en or zh.")
    private String lang;
}
