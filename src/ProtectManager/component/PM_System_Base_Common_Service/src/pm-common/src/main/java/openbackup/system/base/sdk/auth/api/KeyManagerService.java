/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.auth.api;

/**
 * 仅供token生成解密用
 *
 * @author twx1011919
 * @since 2021-05-10
 */
public interface KeyManagerService {
    /**
     * 解密
     *
     * @param text 密文
     * @return 返回解密的明文
     */
    String getDecryptPwd(String text);

    /**
     * 加密
     *
     * @param text 明文
     * @return 返回加密的明文
     */
    String getEncryptPwd(String text);
}
