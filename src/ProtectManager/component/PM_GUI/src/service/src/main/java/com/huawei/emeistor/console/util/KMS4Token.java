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
package com.huawei.emeistor.console.util;

import com.huawei.emeistor.console.bean.CiphertextVo;
import com.huawei.emeistor.console.service.EncryptorRestApi;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.concurrent.atomic.AtomicReference;

/**
 * 仅供token生成解密用
 *
 */
@Component("openbackup.system.utils.common.base.KMS4Token")
public class KMS4Token {
    private static final AtomicReference<KMS4Token> INSTANCE = new AtomicReference<>();

    @Autowired
    private EncryptorRestApi encryptorRestApi;

    /**
     * 无参构造
     */
    public KMS4Token() {
        final KMS4Token previous = INSTANCE.getAndSet(this);
        if (previous != null) {
            throw new IllegalStateException("Second singleton " + this + " created after " + previous);
        }
    }

    public static KMS4Token getInstance() {
        return INSTANCE.get();
    }

    /**
     * 仅供token生成解密用
     *
     * @param text 密文
     * @return 返回解密的明文
     */
    public String decryptText(String text) {
        CiphertextVo ciphertextVo = new CiphertextVo();
        ciphertextVo.setCiphertext(text);
        return encryptorRestApi.decrypt(ciphertextVo).getPlaintext();
    }
}
