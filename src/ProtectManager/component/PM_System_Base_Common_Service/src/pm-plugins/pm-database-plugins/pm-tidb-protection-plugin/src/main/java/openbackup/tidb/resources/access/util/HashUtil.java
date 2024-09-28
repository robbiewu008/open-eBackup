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
package openbackup.tidb.resources.access.util;

import lombok.extern.slf4j.Slf4j;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * HashUtil 工具类
 *
 */
@Slf4j
public final class HashUtil {
    private static final char[] SIXTEEN = new char[] {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
    };

    /**
     * hash 散列
     *
     * @param plain 字符串
     * @return 散列后的值
     */
    public static String digest(String plain) {
        try {
            byte[] plainBytes = plain.getBytes(StandardCharsets.UTF_8);
            MessageDigest md = MessageDigest.getInstance("SHA-256");
            byte[] digestBytes = md.digest(plainBytes);
            return new String(byteToHex(digestBytes));
        } catch (NoSuchAlgorithmException var4) {
            log.error("no algorithm {}", "SHA-256");
            throw new IllegalArgumentException(var4);
        }
    }

    private static char[] byteToHex(byte[] encryptedBytes) {
        char[] result = new char[encryptedBytes.length * 2];

        for (int i = 0; i < encryptedBytes.length; ++i) {
            byte encryptedByte = encryptedBytes[i];
            int high = (240 & encryptedByte) >> 4;
            int low = 15 & encryptedByte;
            result[i * 2] = SIXTEEN[high];
            result[i * 2 + 1] = SIXTEEN[low];
        }
        return result;
    }
}
