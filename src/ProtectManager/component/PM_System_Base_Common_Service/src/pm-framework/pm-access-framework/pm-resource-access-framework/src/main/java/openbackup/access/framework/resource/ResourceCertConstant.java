/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource;

/**
 * 资源证书常量类
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/13
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
