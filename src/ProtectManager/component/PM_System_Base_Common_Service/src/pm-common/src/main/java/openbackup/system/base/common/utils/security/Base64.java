/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.security;

/**
 * Base64工具类
 *
 * @author j90001735
 * @version [UltraAPM V100R002C10, 2013-9-12]
 * @since 2019-10-25
 */
public final class Base64 {
    private static final char[] INT_TO_BASE_64 = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
        'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
    };

    private static final char[] INT_TO_ALT_BASE_64 = {
        '!', '"', '#', '$', '%', '&', '\'', '(', ')', ',', '-', '.', ':', ';', '<', '>', '@', '[', ']', '^', '`', '_',
        '{', '|', '}', '~', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
        's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '?'
    };

    private static final byte[] BASE_64_TO_INT = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59,
        60, 61, -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
        21, 22, 23, 24, 25, -1, -1, -1, -1, -1, -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 51
    };

    private static final byte[] ALT_BASE_64_TO_INT = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, -1, 62, 9, 10, 11, -1, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
        12, 13, 14, -1, 15, 63, 16, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, 17, -1, 18, 19, 21, 20, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42,
        43, 44, 45, 46, 47, 48, 49, 50, 51, 22, 23, 24, 25
    };

    private static final int ONE = 1;

    private static final int TWO = 2;

    private static final int THREE = 3;

    private static final int FOURE = 4;

    private static final int SIX = 6;

    private static final int MASK_255 = 0xFF;

    private static final int MASK_63 = 0x3F;

    private Base64() {
    }

    /**
     * 将字节数组进行Base64编码
     *
     * @param paramArrayOfByte 待编码的字节数组
     * @return String 返回编码后的字符串
     */
    public static String byteArrayToBase64(byte[] paramArrayOfByte) {
        return byteArrayToBase64(paramArrayOfByte, false);
    }

    private static String byteArrayToBase64(byte[] paramArrayOfByte, boolean isAltBase64) {
        int vari = paramArrayOfByte.length;
        int varj = vari / THREE;
        int varm = FOURE * ((vari + TWO) / THREE);
        StringBuilder localStringBuffer = new StringBuilder(varm);
        char[] arrayOfChar = isAltBase64 ? INT_TO_ALT_BASE_64 : INT_TO_BASE_64;
        int index = 0;
        int i2;
        int i1;
        for (i1 = 0; i1 < varj; i1++) {
            i2 = paramArrayOfByte[index++] & MASK_255;
            int i3 = paramArrayOfByte[index++] & MASK_255;
            int i4 = paramArrayOfByte[index++] & MASK_255;
            localStringBuffer.append(arrayOfChar[i2 >> TWO]);
            localStringBuffer.append(arrayOfChar[i2 << FOURE & MASK_63 | i3 >> FOURE]);
            localStringBuffer.append(arrayOfChar[i3 << TWO & MASK_63 | i4 >> SIX]);
            localStringBuffer.append(arrayOfChar[i4 & MASK_63]);
        }

        int vark = vari - THREE * varj;
        if (vark != 0) {
            i1 = paramArrayOfByte[index++] & MASK_255;
            localStringBuffer.append(arrayOfChar[i1 >> TWO]);
            if (vark == 1) {
                localStringBuffer.append(arrayOfChar[i1 << FOURE & MASK_63]);
                localStringBuffer.append("==");
            } else {
                i2 = paramArrayOfByte[index++] & MASK_255;
                localStringBuffer.append(arrayOfChar[i1 << FOURE & MASK_63 | i2 >> FOURE]);
                localStringBuffer.append(arrayOfChar[i2 << TWO & MASK_63]);
                localStringBuffer.append('=');
            }
        }

        return localStringBuffer.toString();
    }

    /**
     * 将Base64编码后的字符串转换为字节数组
     *
     * @param paramString 待转换的字符串
     * @return byte[] 返回转换后的字节数组
     */
    public static byte[] base64ToByteArray(String paramString) {
        return base64ToByteArray(paramString, false);
    }

    private static byte[] base64ToByteArray(String paramString, boolean isAltBase64) {
        int vari = paramString.length();
        int varj = vari / FOURE;
        if (FOURE * varj != vari) {
            throw new IllegalArgumentException("String length must be a multiple of four.");
        }
        int vark = 0;
        int varm = varj;
        if (vari != 0) {
            if (paramString.charAt(vari - ONE) == '=') {
                vark++;
                varm--;
            }
            if (paramString.charAt(vari - TWO) == '=') {
                vark++;
            }
        }
        byte[] arrayOfByte2 = new byte[THREE * varj - vark];

        int index = 0;
        int i1 = 0;
        int i2 = 0;
        int i3;
        int i4;
        byte[] arrayOfByte1 = isAltBase64 ? ALT_BASE_64_TO_INT : BASE_64_TO_INT;
        for (i2 = 0; i2 < varm; i2++) {
            i3 = base64toInt(paramString.charAt(index++), arrayOfByte1);
            i4 = base64toInt(paramString.charAt(index++), arrayOfByte1);
            int i5 = base64toInt(paramString.charAt(index++), arrayOfByte1);
            int i6 = base64toInt(paramString.charAt(index++), arrayOfByte1);
            arrayOfByte2[i1++] = (byte) (i3 << TWO | i4 >> FOURE);
            arrayOfByte2[i1++] = (byte) (i4 << FOURE | i5 >> TWO);
            arrayOfByte2[i1++] = (byte) (i5 << SIX | i6);
        }

        if (vark != 0) {
            i2 = base64toInt(paramString.charAt(index++), arrayOfByte1);
            i3 = base64toInt(paramString.charAt(index++), arrayOfByte1);
            arrayOfByte2[i1++] = (byte) (i2 << TWO | i3 >> FOURE);

            if (vark == 1) {
                i4 = base64toInt(paramString.charAt(index++), arrayOfByte1);
                arrayOfByte2[i1++] = (byte) (i3 << FOURE | i4 >> TWO);
            }
        }

        return arrayOfByte2;
    }

    private static int base64toInt(char paramChar, byte[] paramArrayOfByte) {
        int value = paramArrayOfByte[paramChar];
        if (value < 0) {
            throw new IllegalArgumentException("Illegal character " + paramChar);
        }
        return value;
    }
}
