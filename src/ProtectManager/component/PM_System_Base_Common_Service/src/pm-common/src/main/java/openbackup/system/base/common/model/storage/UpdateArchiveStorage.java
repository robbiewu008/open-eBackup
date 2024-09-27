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
package openbackup.system.base.common.model.storage;

import lombok.Data;

import javax.validation.constraints.NotNull;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-06-29
 */
@Data
public class UpdateArchiveStorage {
    private Integer connectType;

    private String userName;

    private String password;

    private String storageId;

    private String certId;

    private boolean alarmEnable;

    private long alarmThreshold;

    @NotNull
    private String alarmLimitValueUnit;
}
