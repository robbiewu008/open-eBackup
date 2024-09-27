/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.bean;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 在所有节点间同步的文件信息实体类
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-26
 */
@Getter
@Setter
@Builder
@AllArgsConstructor
@NoArgsConstructor
public class FileSyncEntity {
    // 文件的内容
    private String fileContent;

    // 文件的路径带文件名
    private String filePath;

    // 文件类型，TEXT, BINARY
    private String fileType;

    // 文件权限，形如0o640
    private String fileMode;
}
