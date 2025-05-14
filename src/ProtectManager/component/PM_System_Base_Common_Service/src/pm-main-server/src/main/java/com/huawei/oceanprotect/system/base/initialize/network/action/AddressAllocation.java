/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.action;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;

import java.math.BigInteger;
import java.util.ArrayList;
import java.util.List;

/**
 * IP分配规则
 *
 * @author l00347293
 * @since 2020-12-22
 */
public class AddressAllocation {
    private static final String POINT_STRING_DELIMITER = ".";
    private static final String HYPHEN_STRING_DELIMITER = "-";
    private static final String COLON_STRING_DELIMITER = ":";
    private static final int IPV6_CHAR_BIT_MAX_VALUE = 7;
    private static final int IPV6_CHAR_BIT = 4;
    private static final int IPV6_GROUP_BIT = 16;
    private static final int IPV6_GROUP_NUM = 8;
    private static final int IPV6_TOTAL_BIT_NUM = 128;
    private static final long IPV6_GROUP_MAX_VALUE = 0xffffL;
    private static final long INDEX_STRING_FAILED = -1;
    private static final long IPV4_GROUP_MAX_VALUE = 255L;
    private static final long IPV4_GROUP_TOTAL_VALUE = 256L;

    private String startIp;

    private long currentStartIpNum;

    private String endIp;

    private String ipType;

    private BigInteger currentIpv6StartIpNum;

    /**
     * 构造函数中加入起始IP
     *
     * @param ipType 区分IPV4还是IPV6
     * @param startIp 起始Ip
     * @param endIp 结束Ip
     */
    public AddressAllocation(String ipType, String startIp, String endIp) {
        this.startIp = startIp;
        this.endIp = endIp;
        this.ipType = ipType;
        if (InitConfigConstant.IPV6_TYPE_FLAG.equals(ipType)) {
            this.currentIpv6StartIpNum = ipv6ToInt(startIp);
        } else {
            this.currentStartIpNum = getIpFromString(startIp);
        }
    }

    /**
     * 获取指定个数的IP
     *
     * @param count ip个数
     * @return ip列表
     */
    public List<String> getAvailableIps(int count) {
        if (InitConfigConstant.IPV6_TYPE_FLAG.equals(ipType)) {
            List<String> ipList = new ArrayList<>();
            BigInteger ipv6StartIpNum = currentIpv6StartIpNum;
            BigInteger ipv6EndIpNum = ipv6StartIpNum.add(new BigInteger(String.valueOf(count - 1)));
            BigInteger realEndIpNum = ipv6ToInt(endIp);
            if (ipv6EndIpNum.compareTo(realEndIpNum) > 0) {
                return ipList;
            }
            for (int mCount = 0; mCount < count; mCount++) {
                ipList.add(intToIpv6(ipv6StartIpNum.add(new BigInteger(String.valueOf(mCount)))));
            }
            currentIpv6StartIpNum = ipv6EndIpNum.add(new BigInteger(String.valueOf(1)));
            return ipList;
        }
        long startIpNum = currentStartIpNum;
        long endIpNum = startIpNum + count - 1;
        long realEndIpNum = getIpFromString(endIp);
        if (endIpNum > realEndIpNum) {
            return new ArrayList<>();
        }
        List<String> ips = parseIpRange(startIpNum, endIpNum + 1);
        currentStartIpNum = currentStartIpNum + count;
        return ips;
    }

    /**
     * 获取当前IP段总的个数
     *
     * @return ip列表
     */
    public long getAvailableIpCount() {
        long count = 0L;
        if (InitConfigConstant.IPV6_TYPE_FLAG.equals(ipType)) {
            BigInteger ipv6StartIpNum = ipv6ToInt(startIp);
            BigInteger ipv6EndIpNum = ipv6ToInt(endIp);
            count = ipv6EndIpNum.subtract(ipv6StartIpNum).longValue() + 1;
        } else {
            long startIpNum = getIpFromString(startIp);
            long endIpNum = getIpFromString(endIp);
            count = endIpNum - startIpNum + 1;
        }
        if (count <= 0) {
            return 0;
        }
        return count;
    }

    /**
     * 当双控扩容为四控时，IP网段扩容
     * 该函数可重新获取IP段总的个数
     *
     * @param ipRange 原始ip段的范围
     * @param count 所需ip总的个数
     * @return ip列表
     */
    public String getModifyIpNetFromIpRange(String ipRange, long count) {
        String[] ipSegments = ipRange.split(HYPHEN_STRING_DELIMITER);
        String startIpStr = ipSegments[0];
        String endIpStr;
        if (InitConfigConstant.IPV6_TYPE_FLAG.equals(ipType)) {
            BigInteger ipv6StartIpNum = ipv6ToInt(startIpStr);
            BigInteger ipv6EndIpNum = ipv6StartIpNum.add(new BigInteger(String.valueOf(count - 1)));
            endIpStr = intToIpv6(ipv6EndIpNum);
            currentIpv6StartIpNum = ipv6EndIpNum.add(new BigInteger(String.valueOf(1)));
        } else {
            long startIpValue = getIpFromString(startIpStr);
            long endIpValue = startIpValue + count - 1;
            endIpStr = getIpFromLong(endIpValue);
            currentStartIpNum = endIpValue + 1;
        }

        return startIpStr + HYPHEN_STRING_DELIMITER + endIpStr;
    }

    /**
     * 从起始ip段中获取连续的ip段
     *
     * @param count ip个数
     * @return ip段
     */
    public String attainContinusIpNet(int count) {
        String currentStartIp;
        String currentEndIp;
        if (InitConfigConstant.IPV6_TYPE_FLAG.equals(ipType)) {
            BigInteger ipv6StartIpNum = currentIpv6StartIpNum;
            BigInteger ipv6EndIpNum = ipv6StartIpNum.add(new BigInteger(String.valueOf(count - 1)));
            BigInteger realEndIpNum = ipv6ToInt(endIp);
            long ipCount = realEndIpNum.subtract(ipv6EndIpNum).longValue();
            if (ipCount < 0) {
                return "";
            }
            currentStartIp = intToIpv6(ipv6StartIpNum);
            currentEndIp = intToIpv6(ipv6EndIpNum);
            currentIpv6StartIpNum = currentIpv6StartIpNum.add(new BigInteger(String.valueOf(count)));
        } else {
            long startIpNum = currentStartIpNum;
            long endIpNum = startIpNum + count - 1;
            long realEndIpNum = getIpFromString(endIp);
            if (endIpNum > realEndIpNum) {
                return "";
            }
            currentStartIp = getIpFromLong(currentStartIpNum);
            currentEndIp = getIpFromLong(endIpNum);
            currentStartIpNum = currentStartIpNum + count;
        }

        return currentStartIp + HYPHEN_STRING_DELIMITER + currentEndIp;
    }

    /**
     * 获取当前剩下的所有IP
     *
     * @return ip段
     */
    public String attainContinusIpNet() {
        Long availableIpCount = getAvailableIpCount();
        return attainContinusIpNet(availableIpCount.intValue());
    }

    /**
     * 把long类型的Ip转为一般Ip类型：xx.xx.xx.xx
     *
     * @param startIpNum ip起点
     * @param endIpNum ip终点
     * @return ip列表
     */
    private List<String> parseIpRange(long startIpNum, long endIpNum) {
        List<String> ips = new ArrayList<>();
        for (long index = startIpNum; index < endIpNum; index++) {
            ips.add(getIpFromLong(index));
        }
        return ips;
    }

    /**
     * 把long类型的Ip转为一般Ip类型：xx.xx.xx.xx
     *
     * @param ip 整型ip
     * @return Ip字符串
     */
    private String getIpFromLong(Long ip) {
        long s3MaxValue = IPV4_GROUP_TOTAL_VALUE * IPV4_GROUP_MAX_VALUE;
        long s3TotalValue = IPV4_GROUP_TOTAL_VALUE * IPV4_GROUP_TOTAL_VALUE;
        long s2MaxValue = s3TotalValue * IPV4_GROUP_MAX_VALUE;
        long s2TotalValue = s3TotalValue * IPV4_GROUP_TOTAL_VALUE;
        long s1MaxValue = s2TotalValue * IPV4_GROUP_MAX_VALUE;
        String s1 = String.valueOf((ip & s1MaxValue) / s2TotalValue);
        String s2 = String.valueOf((ip & s2MaxValue) / s3TotalValue);
        String s3 = String.valueOf((ip & s3MaxValue) / IPV4_GROUP_TOTAL_VALUE);
        String s4 = String.valueOf(ip & IPV4_GROUP_MAX_VALUE);
        return s1 + POINT_STRING_DELIMITER + s2 + POINT_STRING_DELIMITER + s3 + POINT_STRING_DELIMITER + s4;
    }

    /**
     * 把xx.xx.xx.xx类型的转为long类型的
     *
     * @param ip ip字符串
     * @return ip整型值
     */
    public static long getIpFromString(String ip) {
        long ipLong = 0L;
        String ipTemp = ip;
        ipLong = ipLong * IPV4_GROUP_TOTAL_VALUE + Long.parseLong(ipTemp.substring(0, ipTemp.indexOf('.')));
        ipTemp = ipTemp.substring(ipTemp.indexOf(POINT_STRING_DELIMITER) + 1);
        ipLong = ipLong * IPV4_GROUP_TOTAL_VALUE + Long.parseLong(
            ipTemp.substring(0, ipTemp.indexOf(POINT_STRING_DELIMITER)));
        ipTemp = ipTemp.substring(ipTemp.indexOf(POINT_STRING_DELIMITER) + 1);
        ipLong = ipLong * IPV4_GROUP_TOTAL_VALUE + Long.parseLong(
            ipTemp.substring(0, ipTemp.indexOf(POINT_STRING_DELIMITER)));
        ipTemp = ipTemp.substring(ipTemp.indexOf(POINT_STRING_DELIMITER) + 1);
        return ipLong * IPV4_GROUP_TOTAL_VALUE + Long.parseLong(ipTemp);
    }

    /**
     * ipv6字符串转BigInteger数
     *
     * @param ipv6 ipv6转换为字符串
     * @return ipv6整型值
     */
    public static BigInteger ipv6ToInt(String ipv6) {
        int compressIndex = ipv6.indexOf(COLON_STRING_DELIMITER + COLON_STRING_DELIMITER);
        if (compressIndex != INDEX_STRING_FAILED) {
            String part1s = ipv6.substring(0, compressIndex);
            String part2s = ipv6.substring(compressIndex + 1);
            BigInteger part1 = ipv6ToInt(part1s);
            BigInteger part2 = ipv6ToInt(part2s);
            int part1HasDot = 0;
            char[] charArray = part1s.toCharArray();
            for (char ch : charArray) {
                if (ch == ':') {
                    part1HasDot++;
                }
            }
            return part1.shiftLeft(IPV6_GROUP_BIT * (IPV6_CHAR_BIT_MAX_VALUE - part1HasDot)).add(part2);
        }
        String[] str = ipv6.split(COLON_STRING_DELIMITER);
        BigInteger big = BigInteger.ZERO;
        for (int mCount = 0; mCount < str.length; mCount++) {
            if (str[mCount].isEmpty()) {
                str[mCount] = "0";
            }
            big = big.add(BigInteger.valueOf(Long.valueOf(str[mCount], IPV6_GROUP_BIT))
                .shiftLeft(IPV6_GROUP_BIT * (str.length - mCount - 1)));
        }
        return big;
    }

    /**
     * BigInteger数 转为ipv6字符串
     *
     * @param big ipv6的数值类型
     * @return ipv6字符串类型
     */
    private static String intToIpv6(BigInteger big) {
        BigInteger curBig = big;
        String str = "";
        BigInteger ff = BigInteger.valueOf(IPV6_GROUP_MAX_VALUE);
        for (int mCount = 0; mCount < IPV6_GROUP_NUM; mCount++) {
            str = curBig.and(ff).toString(IPV6_GROUP_BIT) + COLON_STRING_DELIMITER + str;
            curBig = curBig.shiftRight(IPV6_GROUP_BIT);
        }

        // 去掉最后的冒号
        str = str.substring(0, str.length() - 1);
        return str.replaceFirst(":(0+(:)){2,8}",
            COLON_STRING_DELIMITER + COLON_STRING_DELIMITER);
    }

    /**
     * 将精简的ipv6地址扩展为全长度的ipv6地址
     *
     * @param strIpv6 ipv6转换为字符串
     * @return ipv6整型值
     */
    public String completeIpv6(String strIpv6) {
        BigInteger big = ipv6ToInt(strIpv6);
        String str = big.toString(IPV6_GROUP_BIT);
        String completeIpv6Str = "";
        while (str.length() != IPV6_CHAR_BIT * IPV6_CHAR_BIT) {
            str = "0" + str;
        }
        for (int mCount = 0; mCount <= str.length(); mCount += IPV6_CHAR_BIT) {
            completeIpv6Str += str.substring(mCount, mCount + IPV6_CHAR_BIT);
            if ((mCount + IPV6_CHAR_BIT) == str.length()) {
                break;
            }
            completeIpv6Str += COLON_STRING_DELIMITER;
        }
        return completeIpv6Str;
    }

    /**
     * 获取ipv6 subnet base
     *
     * @param strIpv6 ipv6转换为字符串
     * @param length 前缀长度
     * @return ipv6整型值
     */
    public static String subNetBase(String strIpv6, int length) {
        BigInteger curBig = ipv6ToInt(strIpv6);
        curBig = curBig.shiftRight(IPV6_TOTAL_BIT_NUM - length);
        curBig = curBig.shiftLeft(IPV6_TOTAL_BIT_NUM - length);
        return intToIpv6(curBig);
    }

    /**
     * 获取ipv4 subnet
     *
     * @param number 前缀长度
     * @return ipv4整型值
     */
    public static String getSubNetForNumber(String number) {
        String ipMask = null;
        int integer = Integer.parseInt(number);
        int part = integer / 8;
        int remainder = integer % 8;
        int sum = 0;
        for (int i = 8; i > 8 - remainder; i--) {
            sum = sum + (int) Math.pow(2, i - 1);
        }
        if (part == 0) {
            ipMask = sum + ".0.0.0";
        } else if (part == 1) {
            ipMask = "255." + sum + ".0.0";
        } else if (part == 2) {
            ipMask = "255.255." + sum + ".0";
        } else if (part == 3) {
            ipMask = "255.255.255." + sum;
        } else if (part == 4) {
            ipMask = "255.255.255.255";
        } else {
            ipMask = "";
        }
        return ipMask;
    }
}
