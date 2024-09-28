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
package openbackup.system.base.common.constants;

/**
 * 证书相关通用错误码
 *
 */
public final class CertErrorCode {
    /**
     * 原因：目标端的证书已经被吊销。
     * 建议：请替换目标端的证书或删除吊销列表。
     */
    public static final long CERT_IS_REVOKED = 1677931037L;

    /**
     * CA证书已过期或无效
     */
    public static final long CA_CERTIFICATE_IS_INVALID = 1677931024L;

    private CertErrorCode() {
    }
}
