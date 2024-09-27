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

/**
 * 服务类型
 *
 * @author xwx1016404
 * @since 2021-10-10
 */
public enum ServiceType {
    /**
     * SFTP服务
     */
    SFTP("SFTP_SERVICE"),
    /**
     * 标准备份服务
     */
    STANDARD("STANDARD_SERVICE");

    private final String type;

    /**
     * 默认构造函数
     *
     * @param type 服务类型
     */
    ServiceType(String type) {
        this.type = type;
    }

    /**
     * 获取服务类型
     *
     * @return type
     */
    public String getType() {
        return type;
    }
}