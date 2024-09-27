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
package openbackup.system.base.common.scurity;

import openbackup.system.base.config.SystemConfig;

import java.security.KeyStore;

/**
 * 安全证书管理器，用于安全证书的加载
 *
 * @author l90005176
 * @version [OceanStor BCManager V200R001C00, 2016年2月16日]
 * @since 2019-10-25
 */
public final class SecurityCertificateManager {
    /**
     * 证书库密钥
     */
    private static final KeyStoreLoader LOADER =
            new KeyStoreLoader(
                    "PKCS12",
                    SystemConfig.getInstance().getKeystoreFile(),
                    SystemConfig.getInstance()::getKeystorePass);

    /**
     * 默认构造函数
     */
    private SecurityCertificateManager() {}

    /**
     * 根据指定安全证书初始化证书库对象
     *
     * @return KeyStore KeyStore对象
     */
    public static KeyStore getKeyStore() {
        return LOADER.getLastKeyStore();
    }
}
