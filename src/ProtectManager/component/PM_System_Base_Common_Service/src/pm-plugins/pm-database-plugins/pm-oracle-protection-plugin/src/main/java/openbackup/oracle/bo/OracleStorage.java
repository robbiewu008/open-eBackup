/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
