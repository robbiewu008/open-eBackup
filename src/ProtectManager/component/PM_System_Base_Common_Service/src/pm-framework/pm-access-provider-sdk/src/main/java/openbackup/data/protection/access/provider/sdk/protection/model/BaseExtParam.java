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

/**
 * 保护请求体
 *
 */
@Data
public class BaseExtParam {
    /**
     * 首次备份esn
     */
    private String firstBackupEsn;

    /**
     * 上次备份esn
     */
    private String lastBackupEsn;

    /**
     * 优先备份esn
     */
    private String priorityBackupEsn;

    /**
     * 首次备份单元
     */
    private String firstBackupTarget;

    /**
     * 上次备份单元
     */
    private String lastBackupTarget;

    /**
     * 优先备份单元
     */
    private String priorityBackupTarget;

    /**
     * 上次失败esn
     */
    private String failedNodeEsn;

    /**
     * 安全归档
     */
    private boolean enableSecurityArchive;

    /**
     * SLA是否开启worm设置
     */
    private boolean wormSwitch;
}
