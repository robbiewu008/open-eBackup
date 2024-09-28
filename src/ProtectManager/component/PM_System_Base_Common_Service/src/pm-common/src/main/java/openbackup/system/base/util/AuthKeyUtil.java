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

import openbackup.system.base.common.exception.LegoCheckedException;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;

/**
 * 口令工具类
 *
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
