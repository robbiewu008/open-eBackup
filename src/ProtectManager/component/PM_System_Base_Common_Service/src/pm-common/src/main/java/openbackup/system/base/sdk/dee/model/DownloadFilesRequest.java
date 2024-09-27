/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.dee.model;

import lombok.Data;

import java.util.List;

/**
 * 下载副本中的文件的请求体
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-28
 */
@Data
public class DownloadFilesRequest {
    /**
     * 下载路径
     */
    private List<String> paths;

    /**
     * 请求id
     */
    private String requestId;

    /**
     * 副本信息
     */
    private DownLoadCopyInfo copyInfo;

    /**
     * 导出文件id
     */
    private String recordId;
}
