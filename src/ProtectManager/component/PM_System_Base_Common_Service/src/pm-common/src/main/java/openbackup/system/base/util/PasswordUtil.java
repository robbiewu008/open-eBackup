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
package openbackup.system.base.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * 生成密码工具类
 *
 */
@Slf4j
public class PasswordUtil {
    /**
     * 默认密码长度
     */
    public static final int DEFAULT_PW_LENGTH = 18;

    /**
     * data_turbo, oracle_data_turbo, vm_data_turbo密码长度
     */
    public static final int TURBO_PW_LENGTH = 16;

    private static final String LETTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

    private static final String DIGITS = "0123456789";

    private static final String PUNCTUATIONS = "!#$%&()*+,-.:<=>?@[]^_{|}~";

    /**
     * 生成随机组件密码
     *
     * @param length 密码长度
     * @return 密码
     */
    public static String generateComponentPassword(int length) {
        int quotient = length / IsmNumberConstant.THREE;
        int remainder = length % IsmNumberConstant.THREE;
        List<String> passwordList = new ArrayList<>();
        passwordList.addAll(generatePasswordByRandom(quotient, LETTERS));
        passwordList.addAll(generatePasswordByRandom(quotient, DIGITS));
        passwordList.addAll(generatePasswordByRandom(quotient + remainder, PUNCTUATIONS));
        Collections.shuffle(passwordList);
        return String.join("", passwordList);
    }

    /**
     * 使用secure random 从指定字符集生成指定长度密码
     *
     * @param length 密码长度
     * @param charSet 字符集
     * @return 密码
     */
    private static List<String> generatePasswordByRandom(int length, String charSet) {
        try {
            List<String> password = new ArrayList<>();
            int bound = charSet.length();
            SecureRandom secureRandom = SecureRandom.getInstanceStrong();
            for (int i = 0; i < length; i++) {
                int index = secureRandom.nextInt(bound);
                password.add(String.valueOf(charSet.charAt(index)));
            }
            return password;
        } catch (NoSuchAlgorithmException e) {
            log.error("get strong secure random failed", e);
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "get strong secure random failed");
        }
    }
}
