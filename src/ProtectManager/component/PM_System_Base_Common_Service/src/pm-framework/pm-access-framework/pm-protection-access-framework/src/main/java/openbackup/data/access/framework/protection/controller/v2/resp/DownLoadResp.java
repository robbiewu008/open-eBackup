/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.controller.v2.resp;

import lombok.Data;

/**
 * 下载副本中的文件的响应对象
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-28
 */
@Data
public class DownLoadResp {
    private String requestId;

    public DownLoadResp(String requestId) {
        this.requestId = requestId;
    }
}
