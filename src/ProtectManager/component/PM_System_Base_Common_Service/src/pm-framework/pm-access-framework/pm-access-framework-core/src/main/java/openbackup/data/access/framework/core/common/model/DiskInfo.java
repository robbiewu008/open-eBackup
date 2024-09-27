/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Builder;
import lombok.Getter;
import lombok.Setter;

/**
 * 存储盘信息实体类
 *
 * @author z00425178
 * @since 2021-11-13
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
