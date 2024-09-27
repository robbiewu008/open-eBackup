/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 功能描述: FileSystemScrubRequest
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-27
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