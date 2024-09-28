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
package openbackup.system.base.sdk.dee.model;

import lombok.Data;

import java.util.List;

/**
 * 下载副本中的文件的请求体
 *
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
