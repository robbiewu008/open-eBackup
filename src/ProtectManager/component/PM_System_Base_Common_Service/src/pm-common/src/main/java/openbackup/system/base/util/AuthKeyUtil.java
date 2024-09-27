/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.common.exception.LegoCheckedException;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;

/**
 * 口令工具类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-13
 */
public class AuthKeyUtil {
    /**
     * 生成符合要求的随机私钥，12位
     *
     * @return String privateKey
     */
    public static String genPrivateKey() {
        // 必须包含特殊字符，大写字母，小写字母，数字，不能包含超过2个连续相同的字符
        // 特殊字符只使用：#*+-.:?@[]_~
        String[] codes = {"abcdefghijklmnopqrstuvwxyz", "ABCDEFGHIJKLMNOPQRSTUVWXYZ",
            "0123456789", "#*+-.:?@[]_~"};
        SecureRandom secureRandom = null;
        try {
            secureRandom = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            throw new LegoCheckedException("generate agent key failed", e);
        }
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < 3; i++) {
            for (String code : codes) {
                int idx = secureRandom.nextInt(code.length());
                sb.append(code.charAt(idx));
            }
        }
        return sb.toString();
    }
}
