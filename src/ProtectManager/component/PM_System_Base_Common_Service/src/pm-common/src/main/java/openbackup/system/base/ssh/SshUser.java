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
package openbackup.system.base.ssh;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.enums.SshMacs;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.commons.lang3.StringUtils;
import org.apache.sshd.common.NamedFactory;
import org.apache.sshd.common.mac.Mac;

import java.util.List;

/**
 * 功能描述
 *
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
@Slf4j
public class SshUser {
    /**
     * 默认的port
     */
    public static final int SSH_PORT = 22;

    private String ip;

    private String username;

    private String password;

    private boolean isSuperUser = true;

    private String superPassword;

    private int port;

    private int sftpPort;

    private String proxyHost;

    private int proxyPort;

    private String destPath;

    // macs签名算法类型
    // safe 安全算法， compatible 兼容性算法
    private String macsName;

    private List<NamedFactory<Mac>> macTypeList;

    /**
     * 是否sudo免密
     */
    private Boolean isScapeSudoPassword = Boolean.FALSE;

    /**
     * 校验参数
     */
    public void validate() {
        if (port <= 0 && sftpPort <= 0) {
            throw new IllegalArgumentException("port or sftpPort must be greater than 0");
        }
        if (StringUtils.isEmpty(username) || StringUtils.isEmpty(password)) {
            throw new IllegalArgumentException("username or password can't be empty");
        }
    }

    /**
     * 根据关键字初始化一个自定义的 BuiltinMacs 列表
     * 注意在调用此方法之前 请确保macs字段不为正确数值 而不是空
     * 如果找不到也会返回空列表 交由外面判断
     */
    public void initMacs() {
        macTypeList = SshMacs.getMacTypeListByName(macsName);
        if (VerifyUtil.isEmpty(macTypeList)) {
            throw new IllegalArgumentException("cannot find suitable MAC algorithm with name:" + macsName);
        }
        log.info("success to convert mac name:{} to macType List:{}", macsName, macTypeList);
    }
}
