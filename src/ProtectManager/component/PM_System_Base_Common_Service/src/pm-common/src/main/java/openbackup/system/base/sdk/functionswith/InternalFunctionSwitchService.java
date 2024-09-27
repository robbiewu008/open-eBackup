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
package openbackup.system.base.sdk.functionswith;

/**
 * 删除用户功能开关服务（内部调用）
 *
 * @author w30042425
 * @since 2023-01-18
 */
public interface InternalFunctionSwitchService {
    /**
     * 删除指定用户某一功能开关开启情况
     *
     * @param userId 用户ID
     * @return 删除用户功能表的行数
     */
    int delUserFunction(String userId);

    /**
     * 设置指定用户各功能开关开启情况
     *
     * @param userId 用户ID
     * @param canBackUp 备份功能开关开启情况
     * @param canRestore 恢复功能开关开启情况
     * @param canArchive 归档功能开关开启情况
     * @param canReplication 复制功能开关开启情况
     * @return 返回添加/修改数据行数
     */
    String setUserFunction(String userId, boolean canBackUp, boolean canRestore, boolean canArchive,
        boolean canReplication);
}
