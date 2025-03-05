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
package openbackup.data.protection.access.provider.sdk.protection.model;

import lombok.Data;

import java.util.List;

/**
 * 保护请求体
 *
 */
@Data
public class VmExtParam extends BaseExtParam {
    /**
     * 前置执行脚本
     */
    private String preScript;

    /**
     * 后置执行脚本
     */
    private String postScript;

    /**
     * 是否保护全部磁盘
     */
    private boolean allDisk;

    /**
     * 归档是否启用自动索引
     */
    private boolean archiveResAutoIndex;

    /**
     * 备份是否启用自动索引
     */
    private boolean backupResAutoIndex;

    /**
     * 保护VM的磁盘id信息
     */
    private List<String> diskInfo;

    /**
     * 并发数
     */
    private String concurrentRequests;

    /**
     * 并发数uuid
     */
    private String concurrentRequestsUuid;

    /**
     * VMware备份恢复任务指定的代理主机的信息列表
     */
    private List hostList;
}
