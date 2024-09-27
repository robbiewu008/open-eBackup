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
package openbackup.oracle.bo;

import lombok.Getter;
import lombok.Setter;

/**
 * 功能描述
 *
 * @author l30023229
 * @since 2023-11-24
 */
@Getter
@Setter
public class OracleStorage {
    private String username;

    private String password;

    private String ipList;

    private int port;

    // 证书是否开启
    private String enableCert;

    // 证书内容
    private String certification;

    // 证书名称
    private String certName;

    // 证书大小
    private String certSize;

    // 吊销列表内容
    private String revocationList;

    // 吊销列表名称
    private String crmName;

    // 吊销列表大小
    private String cerlSize;
}
