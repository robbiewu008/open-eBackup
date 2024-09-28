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
package openbackup.system.base.common.utils.security;

import openbackup.system.base.sdk.auth.api.KeyManagerService;
import openbackup.system.base.sdk.kmc.EncryptorRestApi;
import openbackup.system.base.sdk.kmc.model.CiphertextVo;
import openbackup.system.base.sdk.kmc.model.PlaintextVo;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.concurrent.atomic.AtomicReference;

/**
 * 加解密
 *
 */
@Component
public class EncryptorUtil implements KeyManagerService {
    private static final AtomicReference<EncryptorUtil> INSTANCE = new AtomicReference<>();

    @Autowired
    private EncryptorRestApi encryptorRestApi;

    /**
     * 无参构造器
     */
    public EncryptorUtil() {
        final EncryptorUtil previous = INSTANCE.getAndSet(this);
        if (previous != null) {
            throw new IllegalStateException("Second singleton " + this + " created after " + previous);
        }
    }

    public static EncryptorUtil getInstance() {
        return INSTANCE.get();
    }

    /**
     * 加密
     *
     * @param plaintext 明文
     * @return 密文
     */
    @Override
    public String getEncryptPwd(String plaintext) {
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext(plaintext);
        return encryptorRestApi.encrypt(plaintextVo).getCiphertext();
    }

    /**
     * 解密
     *
     * @param decryptPwd 密文
     * @return 明文
     */
    @Override
    public String getDecryptPwd(String decryptPwd) {
        CiphertextVo ciphertextVo = new CiphertextVo();
        ciphertextVo.setCiphertext(decryptPwd);
        return encryptorRestApi.decrypt(ciphertextVo).getPlaintext();
    }
}
