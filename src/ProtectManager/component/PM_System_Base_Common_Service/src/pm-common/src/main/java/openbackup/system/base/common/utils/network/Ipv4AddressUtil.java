package openbackup.system.base.common.utils.network;

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.NumberUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.Normalizer;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Ipv4地址工具类
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-12
 */
public class Ipv4AddressUtil {
    private static final Logger LOGGER = LoggerFactory.getLogger(Ipv4AddressUtil.class);

    private static final int VALUE_0X00FFFFFF = 0x00FFFFFF;

    private static final int VALUE_0X0000FFFF = 0x0000FFFF;

    private static final int VALUE_0X000000FF = 0x000000FF;

    private static final Pattern PATTERN_IPV4 = Pattern.compile("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");

    private Ipv4AddressUtil() {
    }

    /**
     * 将127.0.0.1形式的ip地址 转换成 十进制整数形式
     *
     * @param ipaddress ipaddress
     * @return long [返回类型说明]
     */
    public static long ipv4ToLong(String ipaddress) {
        if (VerifyUtil.isEmpty(ipaddress)) {
            return 0;
        }
        long[] ip = new long[LegoNumberConstant.FOUR];
        // 先找到IP地址字符串中.的位置
        int position1 = ipaddress.indexOf(".");
        int position2 = ipaddress.indexOf(".", position1 + 1);
        int position3 = ipaddress.indexOf(".", position2 + 1);
        // 将每个.之间的字符串转换成整型
        ip[0] = NumberUtil.convertToLong(ipaddress.substring(0, position1));
        ip[1] = NumberUtil.convertToLong(ipaddress.substring(position1 + 1, position2));
        ip[LegoNumberConstant.TWO] = NumberUtil.convertToLong(ipaddress.substring(position2 + 1, position3));
        ip[LegoNumberConstant.THREE] = NumberUtil.convertToLong(ipaddress.substring(position3 + 1));
        return (ip[0] << LegoNumberConstant.TWENTY_FOUR) + (ip[1] << LegoNumberConstant.SIXTEEN) + (
            ip[LegoNumberConstant.TWO] << LegoNumberConstant.EIGHT) + ip[LegoNumberConstant.THREE];
    }

    /**
     * 检查所给的是否为一个合法的IP地址 下边三种都是特殊地址，不能用于IP用户 0.* 127.* 255.255.255.255
     * 224.*以上的都是组播或保留地址，也不能用于IP用户。
     * 综合一下，就是：0.*、127.*以及（224～255）.*都不能用于网络上的IP用户。
     *
     * @param ip ip
     * @return true: 合法的IP地址 false: 非法的IP地址
     */
    public static boolean isValidIPv4(String ip) {
        if (isIPv4Address(ip)) {
            String[] iPArray = ip.split("\\.");
            if (iPArray.length != LegoNumberConstant.FOUR) {
                return false;
            }
            int val = NumberUtil.convertToInteger(iPArray[0]);
            return val != LegoNumberConstant.ONE_HUNDRED_TWENTY_SEVEN;
        }
        return false;
    }

    /**
     * IPv4合法性校验
     * 下边三种都是特殊地址，不能用于IP用户: 0.*, 127.*, 255.255.255.255
     * 224.*以上的都是组播或保留地址，也不能用于IP用户。
     * 综合一下，就是：0.*、127.*以及（224～255）.*都不能用于网络上的IP用户。
     *
     * @param ip ip
     * @return true: 合法的IP地址 false: 非法的IP地址
     */
    public static boolean isValidIpv4All(String ip) {
        if (isIPv4Address(ip)) {
            String[] iPArray = ip.split("\\.");
            if (iPArray.length != LegoNumberConstant.FOUR) {
                return false;
            }
            int val = NumberUtil.convertToInteger(iPArray[0]);
            return val != LegoNumberConstant.ZERO && val != LegoNumberConstant.ONE_HUNDRED_TWENTY_SEVEN
                    && val <= LegoNumberConstant.TWO_HUNDRED_FIFTY_THREE;
        }
        return false;
    }

    /**
     * ipv4参数检查
     *
     * @param ip ip
     * @return 参数是否是IPV4地址
     */
    public static boolean isIPv4Address(String ip) {
        if (VerifyUtil.isEmpty(ip)) {
            return false;
        }
        String ipv4 = Normalizer.normalize(ip, Normalizer.Form.NFKC);
        Matcher mat = PATTERN_IPV4.matcher(ipv4);
        if (mat.matches()) {
            try {
                InetAddress val = InetAddress.getByName(ip);
                if (val != null) {
                    return true;
                } else {
                    LOGGER.debug("%s is not a ipv4 address.", ipv4);
                    return false;
                }
            } catch (UnknownHostException ex) {
                LOGGER.debug(ex.getMessage(), ex);
                return false;
            }
        } else {
            LOGGER.debug("%s is not a ipv4 address.", ipv4);
            return false;
        }
    }

    /**
     * 通过子网掩码获取网络段长度
     *
     * @param mask 子网掩码
     * @return 子网掩码网络段长度
     */
    public static int calNetMaskPrefixLen(String mask) {
        String[] octets = mask.split("\\.");
        int totalOnes = 0;

        for (String octet : octets) {
            totalOnes += (8 - Math.log(256 - Integer.valueOf(octet)) / Math.log(2));
        }

        return totalOnes;
    }
}
