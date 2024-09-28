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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.google.common.collect.Lists;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.util.List;

/**
 * 随机生成密码
 *
 */
public class RandomPwdUtil {
    private static final StringBuffer LOWER_CASES = new StringBuffer(
        "a,b,c,d,e,f,g,h,i,g,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z");

    private static final StringBuffer UPPER_CASES = new StringBuffer(
        "A,B,C,D,E,F,G,H,I,G,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z");

    private static final StringBuffer SYMBOLS = new StringBuffer("~,@,#,$,%,^,&,*,(,),!,%");

    private static final StringBuffer NUMBERS = new StringBuffer("1,2,3,4,5,6,7,8,9,0");

    /**
     * 生成指定长度的随机密码
     *
     * @param length 长度
     * @return String
     */
    public static String generate(int length) {
        return generate(length, SYMBOLS);
    }

    /**
     * 生成指定长度的随机密码
     *
     * @param length 长度
     * @param symbols 特殊字符
     * @return String
     */
    public static String generate(int length, StringBuffer symbols) {
        String[] lowerCaseArray = LOWER_CASES.toString().split(",");
        String[] upperCaseArray = UPPER_CASES.toString().split(",");
        String[] symbolArray = symbols.toString().split(",");
        String[] numberArray = NUMBERS.toString().split(",");
        List<String[]> codes = Lists.newArrayList();
        codes.add(lowerCaseArray);
        codes.add(upperCaseArray);
        codes.add(symbolArray);
        codes.add(numberArray);
        return generate(codes, length);
    }

    private static SecureRandom getStrongSecureRandom() {
        SecureRandom random;
        try {
            random = SecureRandom.getInstanceStrong();
        } catch (NoSuchAlgorithmException e) {
            throw LegoCheckedException.cast(
                    e, CommonErrorCode.OPERATION_FAILED, "There is No Strong SecureRandom Algorithm in the System");
        }
        return random;
    }

    private static String generate(List<String[]> codes, int length) {
        StringBuffer stringBuffer = new StringBuffer();
        SecureRandom random = getStrongSecureRandom();
        int key;
        // 先每样选一个
        for (String[] fragment : codes) {
            key = random.nextInt();
            stringBuffer.append(fragment[Math.abs(key % fragment.length)]);
        }
        // 从各个数组中选择数据
        for (int i = 0; i < length - codes.size(); i++) {
            key = random.nextInt();
            int index = key & (codes.size() - 1);
            String[] fragment = codes.get(index);
            stringBuffer.append(fragment[Math.abs(key % fragment.length)]);
        }
        return stringBuffer.toString();
    }

    /**
     * 是否包含相同字符连续3位或3位以上
     *
     * @param password 秘钥
     * @return true 包含 false 不包含
     */
    public static boolean isReduplicate(String password) {
        char[] chars = password.toCharArray();
        for (int i = 0; i < chars.length - 2; i++) {
            char n1 = chars[i];
            char n2 = chars[i + 1];
            char n3 = chars[i + 2];
            // 判断重复字符
            if (n1 == n2 && n1 == n3) {
                return true;
            }
        }
        return false;
    }
}

