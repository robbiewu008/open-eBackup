/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.kerberos.model;

import lombok.Data;

import java.util.Date;

/**
 * KerberosBo
 *
 * @author m00576658
 * @since 2021-08-18
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
