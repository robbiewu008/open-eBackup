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
package openbackup.data.access.framework.core.common.enums;

/**
 * 恢复类型
 *
 * @author p00511147
 * @since 2020-12-29
 */
public enum VmRestoreTypeEnum {
    OVERWRITING("0"),
    SKIP("1"),
    REPLACE("2"),
    DOWNLOAD("3");

    private String mode;

    VmRestoreTypeEnum(String mode) {
        this.mode = mode;
    }

    public String getMode() {
        return this.mode;
    }
}
