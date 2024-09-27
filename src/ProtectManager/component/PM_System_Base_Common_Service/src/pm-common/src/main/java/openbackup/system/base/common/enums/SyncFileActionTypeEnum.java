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
package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 同步文件到所有节点的动作枚举类
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-29
 */
@Getter
public enum SyncFileActionTypeEnum {
    /**
     * 添加
     */
    ADD(0),

    /**
     * 删除
     */
    DELETE(1);

    private final int type;

    SyncFileActionTypeEnum(int type) {
        this.type = type;
    }
}
