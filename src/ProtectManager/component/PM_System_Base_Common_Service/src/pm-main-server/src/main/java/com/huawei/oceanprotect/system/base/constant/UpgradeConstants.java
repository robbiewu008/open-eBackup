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
package com.huawei.oceanprotect.system.base.constant;

import openbackup.system.base.common.enums.DeployTypeEnum;

import org.apache.curator.shaded.com.google.common.collect.ImmutableList;

import java.util.List;

/**
 * 升级常量类
 *
 * @since 2024-06-20
 */
public class UpgradeConstants {
    /**
     * SFTP 用户Dtree NFS路径共享客户端
     */
    public static final String SFTP_USER_DTREE_NFS_CLIENT_NAME = "*";

    /**
     * SFTP 用户Dtree NFS路径租户ID
     */
    public static final String SFTP_USER_DTREE_NFS_CLIENT_VSTORE_ID = "0";

    /**
     * SFTP 用户Dtree NFS路径客户端查询范围
     */
    public static final String SFTP_USER_DTREE_QUERY_RANGE = "[0-100]";

    /**
     * SFTP 部署形态集合
     */
    public static final List<String> SFTP_DEPLOY_TYPE_LIST = ImmutableList.of(DeployTypeEnum.X9000.getValue(),
        DeployTypeEnum.X6000.getValue(), DeployTypeEnum.X8000.getValue());
}
