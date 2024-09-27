/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.util;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;

/**
 * 产生用户登录Session ID
 *
 * @author t00104528
 * @version [LEGO V100R002C01, 2010-8-30]
 * @since 2018-01-01
 */
public final class SessionUtils {
    private static final Logger LOG = LoggerFactory.getLogger(SessionUtils.class);

    private static final int HEX = 255;

    private static final int OCTAL = 32;

    private SessionUtils() {
    }

    /**
     * 产生用户登录的Session id
     *
     * @param userId 用户ID
     * @return String 返回用户登录产生的Session ID
     */
    public static String generateSessionId(final String userId) {
        StringBuffer buffer = new StringBuffer();
        buffer.append("userId=")
            .append(userId)
            .append("-loginTime=")
            .append(System.currentTimeMillis())
            .append(getRandom());
        return buffer.toString();
    }

    private static String getRandom() {
        String hv;
        byte[] bytes = new byte[OCTAL];
        try {
            SecureRandom random = SecureRandom.getInstanceStrong();
            random.nextBytes(bytes);
        } catch (NoSuchAlgorithmException e) {
            LOG.error("Failed to generate random number exception:", e);
        }

        // 将生成的随机数转换成字符串
        StringBuilder stringBuilder = new StringBuilder("");
        for (byte aByte : bytes) {
            hv = Integer.toHexString(aByte & HEX); // 16进制数转换成字符
            if (hv.length() == 1) {
                hv = "0" + hv;
            }
            stringBuilder.append(hv);
        }

        return stringBuilder.toString();
    }
}
