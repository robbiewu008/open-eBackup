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
package openbackup.data.access.framework.livemount.common.enums;

/**
 * Live Mount操作类型枚举类
 *
 * @author tWX1009756
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-28
 */
public enum OperationEnums {
    // 创建即时挂载
    CREATE(0),
    // 修改即时挂载
    MODIFY(1),
    // 更新及时挂载
    UPDATE(2),
    // 取消即时挂载
    CANCEL(3);


    private final int type;

    OperationEnums(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }
}
