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
package openbackup.data.protection.access.provider.sdk.enums;

/**
 * agent执行任务时的挂载方法
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-12
 */
public enum AgentMountTypeEnum {
    /**
     * 普通挂载
     */
    MOUNT("mount"),

    /**
     * fuse挂载（本地盘）
     */
    FUSE("fuse");

    private final String mountType;

    /**
     * agent执行任务时的挂载方法
     *
     * @param mountType 挂载类型
     */
    AgentMountTypeEnum(String mountType) {
        this.mountType = mountType;
    }

    /**
     * 获取枚举值
     *
     * @return 挂载参数
     */
    public String getValue() {
        return mountType;
    }
}
