/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 更新文件系统请求体
 *
 * @author x30028756
 * @since 2022-03-11
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class UpdateFileSystemRequest {
    /**
     * 文件系统list
     */
    private List<ResourceInfo> resourceInfos;

    /**
     * 设备ID
     */
    private String deviceId;
}