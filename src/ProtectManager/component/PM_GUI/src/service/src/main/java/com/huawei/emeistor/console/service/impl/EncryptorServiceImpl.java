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
package com.huawei.emeistor.console.service.impl;

import com.huawei.emeistor.console.bean.CiphertextVo;
import com.huawei.emeistor.console.bean.PlaintextVo;
import com.huawei.emeistor.console.kmchelp.KmcHelper;
import com.huawei.emeistor.console.service.EncryptorRestApi;
import com.huawei.emeistor.console.util.VerifyUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.context.annotation.Primary;
import org.springframework.stereotype.Service;

/**
 * 供common调用KMS加解密实现
 *
 */
@Primary
@Service
@Slf4j
public class EncryptorServiceImpl implements EncryptorRestApi {
    /**
     * 解密
     *
     * @param ciphertextVo 解密对象
     * @return 解密后的产生的对象
     */
    @Override
    public PlaintextVo decrypt(CiphertextVo ciphertextVo) {
        String cipherText = ciphertextVo.getCiphertext();
        String plaintext = VerifyUtil.isEmpty(cipherText) ? cipherText : KmcHelper.getInstance().decrypt(cipherText);
        PlaintextVo plaintextVo = new PlaintextVo();
        plaintextVo.setPlaintext(plaintext);
        return plaintextVo;
    }

    /**
     * 解密
     *
     * @param plaintextVo 加密密对象
     * @return 解密后的产生的对象
     */
    @Override
    public CiphertextVo encrypt(PlaintextVo plaintextVo) {
        String plainText = plaintextVo.getPlaintext();
        String ciphertext = VerifyUtil.isEmpty(plainText) ? plainText : KmcHelper.getInstance().encrypt(plainText);
        CiphertextVo ciphertextVo = new CiphertextVo();
        ciphertextVo.setCiphertext(ciphertext);
        return ciphertextVo;
    }
}
