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
package openbackup.system.base.sdk.cert.request;

import lombok.Getter;
import lombok.Setter;

/**
 * PushUpdateCertToAgentReq
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-11-18
 */
@Getter
@Setter
public class PushUpdateCertToAgentReq {
    /**
     * 任务id
     */
    private String jobId;

    /**
     * pm ca证书
     */
    private String pmCaCertificate;

    /**
     * agent ca 证书
     */
    private String agentCaCertificate;

    /**
     * 服务端证书
     */
    private String serverCertificate;

    /**
     * 服务端私钥
     */
    private String serverKey;

    /**
     * 服务端私钥密码
     */
    private String serverPass;

    /**
     * agent-9  pm-0
     */
    private int type;
}
