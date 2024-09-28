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
package openbackup.data.access.client.sdk.api.storage.model;

import lombok.Data;

import java.util.List;

/**
 * 功能描述
 *
 */
@Data
public class StorageInfo {
    private String ip;

    private Integer port;

    private String username;

    private String password;

    private Integer type;

    /**
     * 管理ip，多个ip以，拼接
     */
    private String managementIps;

    private List<StoragePool> storagePoolList;
}
