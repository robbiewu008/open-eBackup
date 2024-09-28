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
package openbackup.system.base.sdk.kerberos.model;

import lombok.Data;

import java.util.Date;

/**
 * KerberosBo
 *
 */
@Data
public class KerberosBo {
    private String kerberosId;

    private String createModel;

    private String name;

    private String keytabPath;

    private String krb5Path;

    private String principalName;

    private String userId;

    private Date createTime;

    private Date updateTime;

    private String password;

    private String keytabContent;

    private String krb5Content;
}
