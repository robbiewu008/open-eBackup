/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
 * @author f00809938
 * @version OceanCyber 300 1.2.0
 * @since 2024-02-23
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
