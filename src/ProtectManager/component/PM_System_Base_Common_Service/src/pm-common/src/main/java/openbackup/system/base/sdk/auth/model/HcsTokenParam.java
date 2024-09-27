/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import openbackup.system.base.sdk.auth.model.request.Scope;

import lombok.Getter;
import lombok.Setter;

/**
 * 获取hcs的token参数
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/7/27
 */
@Getter
@Setter
public class HcsTokenParam {
    // hcs的域名对应的ip地址
    private String ip;

    // hcs的vdc管理员用户名
    private String username;

    // hcs的vdc管理员密码
    private String password;

    // hcs对外全局域名
    private String globalDomain;

    // hcs的token作用域名称
    private String domainName;

    // hcs的token作用域
    private Scope scope;
}
