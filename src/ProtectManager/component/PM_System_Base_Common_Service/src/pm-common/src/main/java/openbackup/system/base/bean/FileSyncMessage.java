/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.bean;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 文件在所有节点间同步的kafka消息
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-29
 */
@Getter
@Setter
public class FileSyncMessage {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 文件信息实体类列表
     */
    private List<FileSyncEntity> fileSyncEntityList;

    /**
     * 0:ADD,1:DELETE
     */
    private Integer action;
}
