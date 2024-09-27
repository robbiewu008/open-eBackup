/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.common.constants;

/**
 * hcs复制常量类
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/7/26
 */
public class HcsConstant {
    /**
     * hcs运营面管理员token的前缀
     */
    public static final String OTHER_HCS_TOKEN = "other_hcs_token";

    /**
     * iam鉴权网关
     */
    public static final String IAM_AUTH_PRFFIX = "iam-apigateway-proxy.";

    /**
     * https前缀
     */
    public static final String HTTPS_PRIFFIX = "https://";

    /**
     * sc域名前缀
     */
    public static final String SC_PRFFIX = "sc.";

    /**
     * 获取hcs运营面管理员token的域名
     */
    public static final String DOMAIN_NAME = "mo_bss_admin";

    /**
     * 修改域名的文件
     */
    public static final String HOSTS_FILE_PATH = "/etc/hosts";

    /**
     * linux执行命令成功响应码
     */
    public static final int SUCCESS_CODE = 0;

    /**
     * 更新hosts的脚本
     */
    public static final String PYTHON_SCRIPT_PATH = "/script/update_hosts.py";
}
