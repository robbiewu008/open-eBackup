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
package openbackup.system.base.sdk.auth.model.dao;

import static com.baomidou.mybatisplus.annotation.IdType.INPUT;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * 预置账号密码Po类
 *
 * @author y30021475
 * @since 2023-08-07
 */
@Getter
@Setter
@TableName(value = PresetAccountPo.TABLE_NAME)
public class PresetAccountPo {
    /**
     * table name
     */
    public static final String TABLE_NAME = "t_external_account_info";

    @TableId(type = INPUT)
    private String uuid;

    private String userName;

    private String userPwd;

    private String sourceType;
}
