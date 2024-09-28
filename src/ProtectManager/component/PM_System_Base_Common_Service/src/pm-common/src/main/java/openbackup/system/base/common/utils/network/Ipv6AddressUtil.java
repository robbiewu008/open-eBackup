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

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import lombok.extern.slf4j.Slf4j;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.math.BigInteger;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.Normalizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Ipv6地址工具类
 *
 */
@Slf4j
public final class Ipv6AddressUtil {
    private static final Logger LOGGER = LoggerFactory.getLogger(Ipv6AddressUtil.class);

    private static final Pattern PATTERN_IPV6 = Pattern.compile(AddressUtil.IPV6REG);

    private Ipv6AddressUtil() {}

    /**
     * 返回ip的种类名称, ipv4或者ipv6, 如果ip不合法, 则返回""
     *
     * @param serverIp ip地址字符串
     * @return 返回ip类型
     */
    public static String convertIpv6(String serverIp) {
        if (isSavedIpv6(serverIp)) {
            return serverIp;
        }
        return "[" + serverIp + "]";
    }

    /**
     * 查看
     *
     * @param serverIp ip地址字符串
     * @return 返回ip类型
     */
    public static boolean isSavedIpv6(String serverIp) {
        return serverIp.startsWith("[") && serverIp.endsWith("]");
    }

    /**
     * 将fc00::1234形式的ip地址转换成十进制整数形式
     *
     * @param ip ip address
     * @return BigInteger [返回类型说明]
     */
    public static BigInteger ipv6ToBigInteger(String ip) {
        String ipAddress = ip;
        byte[] byteArr = new byte[LegoNumberConstant.SEVENTEEN];
        byteArr[0] = 0;
        int ib = LegoNumberConstant.SIXTEEN;
        boolean isMixFlag = false; // IPv4混合模式标记
        if (ipAddress.startsWith("::")) { // 以“::”开头
            ipAddress = ipAddress.substring(2);
        } else if (ipAddress.endsWith("::")) { // 以“::”结尾
            ipAddress = ipAddress + "0";
        } else {
            log.info("ipaddress is not have ::");
        }
        String[] groups = ipAddress.split(":");
        // 反向扫描
        for (int ig = groups.length - 1; ig > -1; ig--) {
            if (groups[ig].contains(".")) {
                // 出现IPv4混合模式
                String ipv4Addr = groups[ig];
                byte[] ipv4ByteArr = new byte[LegoNumberConstant.FIVE];
                ipv4ByteArr[0] = 0;
                // 先找到IP地址字符串中“.”的位置
                int position1 = ipv4Addr.indexOf(".");
                int position2 = ipv4Addr.indexOf(".", position1 + 1);
                int position3 = ipv4Addr.indexOf(".", position2 + 1);
                // 将每个“.”之间的字符串转换成整型
                ipv4ByteArr[1] = (byte) Integer.parseInt(ipv4Addr.substring(0, position1));
                ipv4ByteArr[2] = (byte) Integer.parseInt(ipv4Addr.substring(position1 + 1, position2));
                ipv4ByteArr[3] = (byte) Integer.parseInt(ipv4Addr.substring(position2 + 1, position3));
                ipv4ByteArr[4] = (byte) Integer.parseInt(ipv4Addr.substring(position3 + 1));
                byteArr[ib--] = ipv4ByteArr[4];
                byteArr[ib--] = ipv4ByteArr[3];
                byteArr[ib--] = ipv4ByteArr[2];
                byteArr[ib--] = ipv4ByteArr[1];
                isMixFlag = true;
            } else if ("".equals(groups[ig])) {
                // 出现零长度压缩,计算缺少的组数
                int zlg = LegoNumberConstant.NINE - (groups.length + (isMixFlag ? 1 : 0));
                // 将这些组置0
                while (zlg-- > 0) {
                    byteArr[ib--] = 0;
                    byteArr[ib--] = 0;
                }
            } else {
                int temp = Integer.parseInt(groups[ig], LegoNumberConstant.SIXTEEN);
                byteArr[ib--] = (byte) temp;
                byteArr[ib--] = (byte) (temp >> LegoNumberConstant.EIGHT);
            }
        }
        return new BigInteger(byteArr);
    }

    /**
     * 检查所给的是否为一个合法的IP地址 下边三种都是特殊地址，
     * 组播或保留地址，也不能用于IP用户。
     *
     * @param ipv6 ip
     * @return true: 合法的IP地址 false: 非法的IP地址
     */
    public static boolean isValidIpv6(String ipv6) {
        if (isIpv6Address(ipv6)) {
            try {
                InetAddress address = InetAddress.getByName(ipv6);
                if (address == null) {
                    return false;
                } else {
                    return !address.isMulticastAddress()
                            && !address.isLoopbackAddress()
                            && !address.isAnyLocalAddress();
                }
            } catch (UnknownHostException e) {
                LOGGER.error("check isValidIPV6 failed, error: ", ExceptionUtil.getErrorMessage(e));
                return false;
            }
        } else {
            return Ipv4AddressUtil.isValidIPv4(ipv6);
        }
    }

    /**
     * ipv6参数检查
     *
     * @param ip ip...
     * @return 参数是否是IPV6地址
     */
    public static boolean isIpv6Address(String ip) {
        if (VerifyUtil.isEmpty(ip)) {
            LOGGER.debug("ip is null.");
            return false;
        }
        String ipv6 = Normalizer.normalize(ip, Normalizer.Form.NFKC);
        Matcher mat = PATTERN_IPV6.matcher(ipv6);
        if (mat.matches()) {
            try {
                InetAddress address = InetAddress.getByName(ipv6);
                return address != null;
            } catch (UnknownHostException ex1) {
                LOGGER.debug("UnknownHostException: ", ExceptionUtil.getErrorMessage(ex1));
                return false;
            }
        } else {
            LOGGER.debug("%s :is not a ipv6 address. address: {}", ipv6);
            return false;
        }
    }
}
