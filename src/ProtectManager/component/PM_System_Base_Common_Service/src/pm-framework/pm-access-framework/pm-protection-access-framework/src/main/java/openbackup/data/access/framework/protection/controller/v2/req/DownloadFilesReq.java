/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.controller.v2.req;

import lombok.Data;

import java.util.List;

/**
 * The DownloadFilesReq
 *
 * @author z00633516
 * @since 2022/03/03
 */
@Data
public class DownloadFilesReq {
    private String recordId;

    private List<String> paths;
}
