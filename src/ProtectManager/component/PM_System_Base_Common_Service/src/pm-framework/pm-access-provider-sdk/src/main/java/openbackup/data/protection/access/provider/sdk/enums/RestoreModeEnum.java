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

import lombok.Getter;

/**
 * 恢复模式
 *
 */
public enum RestoreModeEnum {
    /**
     * 远端副本先下载到本地再恢复
     */
    DOWNLOAD_RESTORE("DownloadRestore"),

    /**
     * 远端副本直接恢复
     */
    REMOTE_RESTORE("RemoteRestore"),

    /**
     * 本地副本直接恢复
     */
    LOCAL_RESTORE("LocalRestore");

    @Getter
    private final String mode;

    /**
     * 构造方法
     *
     * @param mode 恢复模式
     */
    RestoreModeEnum(String mode) {
        this.mode = mode;
    }
}
