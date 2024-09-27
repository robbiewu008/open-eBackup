/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.constants;

/**
 * 证书相关通用错误码
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/8/2
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
