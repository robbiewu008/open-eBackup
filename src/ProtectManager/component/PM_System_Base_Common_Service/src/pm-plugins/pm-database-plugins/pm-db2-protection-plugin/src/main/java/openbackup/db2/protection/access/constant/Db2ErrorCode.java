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
package openbackup.db2.protection.access.constant;

/**
 * db2错误码
 *
 */
public class Db2ErrorCode {
    /**
     * 场景: 执行创建/修改保护对象操作时，由于所选保护对象子资源数量超过系统上限，操作失败。
     * 原因: 所选保护对象子资源数量超过系统上限（{0}）。
     * 建议: 请删除不再使用的保护对象子资源后重试。
     */
    public static final long CHECK_RESOURCES_SIZE_ERROR = 1577209985L;
}
