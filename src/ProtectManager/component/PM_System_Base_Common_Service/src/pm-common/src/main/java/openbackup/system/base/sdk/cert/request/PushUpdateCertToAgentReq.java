/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
