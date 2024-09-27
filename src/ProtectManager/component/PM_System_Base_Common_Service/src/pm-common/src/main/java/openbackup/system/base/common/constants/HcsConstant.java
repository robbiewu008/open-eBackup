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
