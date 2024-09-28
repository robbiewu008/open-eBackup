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
package openbackup.system.base.common.utils.network;

import java.util.Arrays;
import java.util.Locale;
import java.util.regex.Pattern;

/**
 * IPv6AddressHelpers
 *
 */
public final class IPv6AddressHelpers {
    private static final Pattern DOT_DELIM = Pattern.compile("\\.");

    /**
     * parseStringArrayIntoLongArray
     *
     * @param strings strings
     * @return long[]
     */
    static long[] parseStringArrayIntoLongArray(String[] strings) {
        final long[] longs = new long[strings.length];
        for (int i = 0; i < strings.length; i++) {
            longs[i] = Long.parseLong(strings[i], 16);
        }
        return longs;
    }

    /**
     * validateLongs
     *
     * @param longs longs
     */
    static void validateLongs(long[] longs) {
        if (longs.length != 8) {
            throw new IllegalArgumentException(
                "an IPv6 address should contain 8 shorts [" + Arrays.toString(longs) + "]");
        }

        for (long value : longs) {
            if (value < 0) {
                throw new IllegalArgumentException("each element should be positive [" + Arrays.toString(longs) + "]");
            }
            if (value > 0xFFFF) {
                throw new IllegalArgumentException(
                    "each element should be less than 0xFFFF [" + Arrays.toString(longs) + "]");
            }
        }
    }

    /**
     * mergeLongArrayIntoIPv6Address
     *
     * @param longs longs
     * @return IPv6Address
     */
    static IPv6Address mergeLongArrayIntoIPv6Address(long[] longs) {
        long high = 0L;
        long low = 0L;

        for (int i = 0; i < longs.length; i++) {
            if (inHighRange(i)) {
                high |= (longs[i] << ((4 - i - 1) * 16));
            } else {
                low |= (longs[i] << ((4 - i - 1) * 16));
            }
        }

        return new IPv6Address(high, low);
    }

    /**
     * inHighRange
     *
     * @param shortNumber shortNumber
     * @return boolean
     */
    static boolean inHighRange(int shortNumber) {
        return shortNumber >= 0 && shortNumber < 4;
    }

    /**
     * expandShortNotation
     *
     * @param notation notation
     * @return String
     */
    static String expandShortNotation(String notation) {
        if (!notation.contains("::")) {
            return notation;
        } else if ("::".equals(notation)) {
            return generateZeroes(8);
        } else {
            final int numberOfColons = countOccurrences(notation, ':');
            if (notation.startsWith("::")) {
                return notation.replace("::", generateZeroes((7 + 2) - numberOfColons));
            } else if (notation.endsWith("::")) {
                return notation.replace("::", ":" + generateZeroes((7 + 2) - numberOfColons));
            } else {
                return notation.replace("::", ":" + generateZeroes((7 + 2 - 1) - numberOfColons));
            }
        }
    }

    /**
     * Replaces a w.x.y.z substring at the end of the given string
     * with corresponding hexadecimal notation. This is useful in case the
     * string was using IPv4-Mapped address notation.
     *
     * @param string string
     * @return String
     */
    static String rewriteIPv4MappedNotation(String string) {
        if (!string.contains(".")) {
            return string;
        } else {
            int lastColon = string.lastIndexOf(":");
            String firstPart = string.substring(0, lastColon + 1);
            String mappedIPv4Part = string.substring(lastColon + 1);

            if (mappedIPv4Part.contains(".")) {
                String[] dotSplits = DOT_DELIM.split(mappedIPv4Part);
                if (dotSplits.length != 4) {
                    throw new IllegalArgumentException(String.format(Locale.ROOT, "can not parse [%s]", string));
                }

                StringBuilder rewrittenString = new StringBuilder();
                rewrittenString.append(firstPart);
                int byteZero = Integer.parseInt(dotSplits[0]);
                int byteOne = Integer.parseInt(dotSplits[1]);
                int byteTwo = Integer.parseInt(dotSplits[2]);
                int byteThree = Integer.parseInt(dotSplits[3]);

                rewrittenString.append(String.format(Locale.ROOT, "%02x", byteZero));
                rewrittenString.append(String.format(Locale.ROOT, "%02x", byteOne));
                rewrittenString.append(":");
                rewrittenString.append(String.format(Locale.ROOT, "%02x", byteTwo));
                rewrittenString.append(String.format(Locale.ROOT, "%02x", byteThree));

                return rewrittenString.toString();
            } else {
                throw new IllegalArgumentException(String.format(Locale.ROOT, "can not parse [%s]", string));
            }
        }
    }

    /**
     * countOccurrences
     *
     * @param haystack haystack
     * @param needle needle
     * @return int
     */
    public static int countOccurrences(String haystack, char needle) {
        int count = 0;
        for (int i = 0; i < haystack.length(); i++) {
            if (haystack.charAt(i) == needle) {
                count++;
            }
        }
        return count;
    }

    /**
     * generateZeroes
     *
     * @param number number
     * @return String
     */
    public static String generateZeroes(int number) {
        final StringBuilder builder = new StringBuilder();
        for (int i = 0; i < number; i++) {
            builder.append("0:");
        }

        return builder.toString();
    }

    /**
     * isLessThanUnsigned
     *
     * @param first 第一个操作数
     * @param second 第二个操作数
     * @return boolean
     */
    static boolean isLessThanUnsigned(long first, long second) {
        return (first < second) ^ ((first < 0) != (second < 0));
    }

    /**
     * prefixWithZeroBytes
     *
     * @param original original
     * @param newSize newSize
     * @return byte[]
     */
    static byte[] prefixWithZeroBytes(byte[] original, int newSize) {
        byte[] target = new byte[newSize];
        System.arraycopy(original, 0, target, newSize - original.length, original.length);
        return target;
    }
}
