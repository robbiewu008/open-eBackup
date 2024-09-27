/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.ssh;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import org.apache.commons.lang3.StringUtils;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-10-07
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
@Builder
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
}
