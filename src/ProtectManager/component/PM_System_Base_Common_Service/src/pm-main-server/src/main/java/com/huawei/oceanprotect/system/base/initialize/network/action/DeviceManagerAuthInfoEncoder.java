/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
