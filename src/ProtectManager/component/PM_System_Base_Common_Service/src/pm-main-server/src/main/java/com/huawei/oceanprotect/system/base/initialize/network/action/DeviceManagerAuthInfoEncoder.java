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
package com.huawei.oceanprotect.system.base.initialize.network.action;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerInfoEncoder;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

/**
 * DeviceManager认证信息编码器
 *
 * @author w00493811
 * @since 2021-05-05
 */
@Service("deviceManagerAuthInfoEncoder")
public class DeviceManagerAuthInfoEncoder implements DeviceManagerInfoEncoder {
    @Autowired
    private EncryptorService encryptorService;

    @Override
    public String encrypt(String plaintext) {
        return encryptorService.encrypt(plaintext);
    }

    @Override
    public String decrypt(String ciphertext) {
        return encryptorService.decrypt(ciphertext);
    }
}
