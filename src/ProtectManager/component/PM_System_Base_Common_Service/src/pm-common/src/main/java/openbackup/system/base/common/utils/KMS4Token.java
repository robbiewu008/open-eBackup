/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.sdk.auth.api.KeyManagerService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.concurrent.atomic.AtomicReference;

/**
 * 仅供token生成解密用
 *
 * @author twx1011919
 * @since 2021-05-10
 */
@Component("openbackup.system.utils.common.base.KMS4Token")
public class KMS4Token {
    private static final AtomicReference<KMS4Token> INSTANCE = new AtomicReference<>();

    @Autowired
    private KeyManagerService keyManagerService;

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
        return keyManagerService.getDecryptPwd(text);
    }
}
