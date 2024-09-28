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
package openbackup.data.access.framework.restore.constant;

/**
 * 恢复任务中单独的码定义
 *
 **/
public abstract class RestoreTaskErrorCode {
    /**
     * 错误场景：执行恢复任务时，由于恢复的目标环境不存在，操作失败。</br>
     *
     * 原因：恢复的目标环境（{0}）不存在。</br>
     * 建议：</br>
     * 1.如果是原位置恢复，当前副本的环境信息不存在，请使用新位置恢复。</br>
     * 2.如果是新位置恢复，选择的目标环境信息不存在，请重试选择最新的目标环境。</br>
     */
    public static final long RESTORE_TARGET_ENV_NOT_EXISTED = 1677933068L;

    private RestoreTaskErrorCode() {
    }
}
