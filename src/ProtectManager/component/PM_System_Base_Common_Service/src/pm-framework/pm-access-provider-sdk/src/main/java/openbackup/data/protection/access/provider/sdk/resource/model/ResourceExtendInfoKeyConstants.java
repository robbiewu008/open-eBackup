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
package openbackup.data.protection.access.provider.sdk.resource.model;

/**
 * resource extend info的内置key
 *
 */
public class ResourceExtendInfoKeyConstants {
    /**
     * 受信key，表示该资源是否守信
     * 用于主机
     *
     * 取值 "true" "false"
     */
    public static final String TRUSTWORTHINESS = "trustworthiness";

    /**
     * agent扩展信息中区分内外置类型的字段
     */
    public static final String EXT_INFO_SCENARIO = "scenario";

    /**
     * agent注册用户id
     */
    public static final String REGISTER_USER_ID = "register_user_id";

    /**
     * 管理数据恢复内置agent注册版本
     */
    public static final String RECOVER_REGISTER_VERSION = "recover_register_version";

    /**
     * 管理数据恢复内置agent注册版本
     */
    public static final String SYSTEM_RECOVER_VERSION = "system_recover_version";
}
