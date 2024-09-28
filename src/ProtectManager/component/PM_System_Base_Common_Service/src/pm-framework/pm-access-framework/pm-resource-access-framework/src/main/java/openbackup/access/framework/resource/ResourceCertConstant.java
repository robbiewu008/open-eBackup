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
package openbackup.access.framework.resource;

/**
 * 资源证书常量类
 *
 */
public final class ResourceCertConstant {
    /**
     * 吊销列表过期告警
     */
    public static final String CRL_EXPIRED_ID = "0x6403320006";

    /**
     * 证书过期告警
     */
    public static final String CERT_EXPIRED_ID = "0x6403320007";

    private ResourceCertConstant() {}
}
