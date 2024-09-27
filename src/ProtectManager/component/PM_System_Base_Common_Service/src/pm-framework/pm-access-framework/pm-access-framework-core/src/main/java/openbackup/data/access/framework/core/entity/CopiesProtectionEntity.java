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
package openbackup.data.access.framework.core.entity;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Builder;
import lombok.Data;

/**
 * 功能描述
 *
 * @author l30044826
 * @since 2024-01-29
 */
@Data
@Builder
@TableName("COPIES_PROTECTION")
public class CopiesProtectionEntity {
    @TableId
    private String protectedResourceId;

    private String protectedObjectUuid;

    private String protectedSlaId;

    private String protectedSlaName;

    private Boolean protectedStatus;

    private String protectedType;

    private String protectedSubType;

    private String protectedChainId;
}
