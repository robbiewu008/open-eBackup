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
package openbackup.system.base.common.model.hcs;

import lombok.Getter;
import lombok.Setter;

/**
 * 功能描述 磁盘信息
 *
 * @author z30027603
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/13 17:54
 */
@Getter
@Setter
public class DiskInfo {
    private String uuid;

    private String id;

    private String name;

    private String mode;

    private String attr;

    private String size;

    private String lunWWN;

    private String shareable;

    private String architecture;

    private String serverId;

    private String sn;

    private String storageManagerIp;

    private String storageType;

    /**
     * 是否是加密盘
     */
    private String systemEncrypted;

    private String systemCmkId;

    private String cipher;
}
