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

import com.huawei.emeistor.console.exterattack.ExterAttack;

import org.apache.commons.codec.binary.Hex;
import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RBucket;
import org.redisson.api.RedissonClient;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.util.Arrays;

import javax.annotation.PostConstruct;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;

/**
 * sha256加密类
 *
 * @author jwx701567
 * @since 2021-09-22
 */
@Component
public class SHA256Encryptor {
    private static final Logger LOG = LoggerFactory.getLogger(SHA256Encryptor.class);

    private static final int SALT_BYTE_SIZE = 16;

    // 会话id前缀
    private static final int SESSION_ID_PREFIX_LENGTH = "userId=88a94c476f12a21e016f12a246e50009-loginTime=".length();

    private static final int ZERO_LENGTH = 0;

    // KEY长度
    private static final int HASH_BYTE_SIZE = 256;

    private static final String SESSION_KEY = "SESSION_KEY";

    // 迭代次数
    private static final int ITERATIONS = 50000;

    // 算法名称
    private static final String PBKDF2_ALGORITHM = "PBKDF2WithHmacSHA256";

    @Autowired
    private RedissonClient redissonClient;

    private SHA256Encryptor() {
    }

    /**
     * 初始化
     */
    @PostConstruct
    public void initSessionSalt() {
        try {
            RBucket<String> rb = redissonClient.getBucket(SESSION_KEY);
            if (!rb.isExists()) {
                LOG.info("init sever Generates a session salt value");
                rb.set(getSalt());
            }
        } catch (Exception e) {
            LOG.error("init session salt error");
        }
    }

    /**
     * 生成一个随机盐值
     *
     * @return String 随机盐值
     */
    public String getSalt() {
        byte[] salt = new byte[0];
        try {
            SecureRandom random = SecureRandom.getInstanceStrong();
            salt = new byte[SALT_BYTE_SIZE];
            random.nextBytes(salt);
        } catch (NoSuchAlgorithmException e) {
            LOG.error("Failed to generate random number exception:", e);
        }
        return Arrays.toString(salt);
    }

    private String encryptWithSalt(String password, String salt) {
        String enCode = "";
        try {
            enCode = getHashCode(password.toCharArray(), salt.getBytes(Charset.defaultCharset()));
        } catch (InvalidKeySpecException e) {
            LOG.error("InvalidKeySpecException failed exception: ", e);
        } catch (Exception e) {
            LOG.error("encryptWithSalt failed");
        }

        return enCode;
    }

    @ExterAttack
    private String encryptWithSalt(String password) {
        RBucket<String> rb = redissonClient.getBucket(SESSION_KEY);
        if (!rb.isExists()) {
            LOG.info("Generates a session salt value");
            rb.set(getSalt());
        }
        return encryptWithSalt(password, rb.get());
    }

    private String getHashCode(char[] password, byte[] salt)
            throws NoSuchAlgorithmException, InvalidKeySpecException {
        PBEKeySpec spec = new PBEKeySpec(password, salt, ITERATIONS, HASH_BYTE_SIZE);
        SecretKeyFactory skf = SecretKeyFactory.getInstance(PBKDF2_ALGORITHM);
        byte[] hash = skf.generateSecret(spec).getEncoded();
        return Hex.encodeHexString(hash);
    }

    /**
     * 加密sessionId
     * sessionId的前缀信息保持不变，对后面的随机值进行加密。
     * userId=88a94c476f12a21e016f12a246e50009-loginTime=
     * 16287518749506ec1d17d109b9b90906936ef435b10f29e48d2996cc5906a386015b79d45673c
     *
     * @param sessionId sessionId
     * @return 加密得sessionId
     */
    public String encryptionSessionId(String sessionId) {
        if (StringUtils.isBlank(sessionId)) {
            return "";
        }
        return sessionId.substring(ZERO_LENGTH, SESSION_ID_PREFIX_LENGTH)
                + encryptWithSalt(sessionId.substring(SESSION_ID_PREFIX_LENGTH));
    }
}
