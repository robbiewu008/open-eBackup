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

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.charset.Charset;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;

import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

/**
 * sha256加密类
 *
 */
public class SHA256Encryptor {
    // 盐值长度
    private static final int SALT_BYTE_SIZE = LegoNumberConstant.SIXTEEN;

    // KEY长度
    private static final int HASH_BYTE_SIZE = 256;

    // 迭代次数
    private static final int ITERATIONS = 50000;

    // 算法名称
    private static final String PBKDF2_ALGORITHM = "PBKDF2WithHmacSHA256";

    private static final Logger LOG = LoggerFactory.getLogger(SHA256Encryptor.class);

    private SHA256Encryptor() {
    }

    /**
     * 为用户指定一个随机盐值
     * 新增或修改用户时，需要调用此方法给用户指定一个随机盐值
     *
     * @return String 随机盐值
     */
    public static String getSalt() {
        try {
            SecureRandom random = SecureRandom.getInstanceStrong();
            byte[] salt = new byte[SALT_BYTE_SIZE];
            random.nextBytes(salt);
            return Arrays.toString(salt);
        } catch (Exception e) {
            LOG.error("get salt failed");
            throw new EmeiStorDefaultExceptionHandler(e.toString());
        }
    }

    /**
     * 使用盐值加密
     *
     * @param password 当前用户密码
     * @param salt     当前用户的盐值
     * @return String 返回hashCode
     */
    public static String encryptWithSalt(String password, String salt) {
        String enCode = "";
        if (VerifyUtil.isEmpty(salt)) {
            LOG.error("the salt is null, encrypt failed.");
            return enCode;
        }

        try {
            enCode = getHashCode(password.toCharArray(), salt.getBytes(Charset.defaultCharset()));
        } catch (Exception e) {
            LOG.error("encryptWithSalt failed: %s", ExceptionUtil.getErrorMessage(e));
        }

        return enCode;
    }

    private static String getHashCode(char[] password, byte[] salt)
        throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, HASH_BYTE_SIZE);
        SecretKeyFactory skf = SecretKeyFactory.getInstance(PBKDF2_ALGORITHM);
        byte[] hash = skf.generateSecret(spec).getEncoded();

        return Arrays.toString(hash);
    }
}
