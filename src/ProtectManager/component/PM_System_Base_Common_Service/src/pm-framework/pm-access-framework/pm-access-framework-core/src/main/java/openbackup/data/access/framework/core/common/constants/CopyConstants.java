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
package openbackup.data.access.framework.core.common.constants;

/**
 * 副本处理过程中需要的常量
 *
 **/
public class CopyConstants {
    /**
     * 副本id
     */
    public static final String BACKUP_ID = "backup_id";

    /**
     * 副本是否过期
     */
    public static final String COPY_EXPIRE = "COPY_EXPIRE";

    /**
     * 成功状态码
     */
    public static final String SUCCESS_CODE = "0";

    /**
     * chainId
     */
    public static final String CHAIN_ID = "chain_id";

    /**
     * ebackup copy
     */
    public static final String E_BACKUP_COPY = "eBackup_copy";

    /**
     * resource
     */
    public static final String RESOURCE = "resource";

    /**
     * "sla"
     */
    public static final String SLA = "sla";

    /**
     * "policy"备份策略
     */
    public static final String POLICY = "policy";

    /**
     * 副本名称在redis 里面的key
     */
    public static final String COPY_NAME = "copy_name";
}
