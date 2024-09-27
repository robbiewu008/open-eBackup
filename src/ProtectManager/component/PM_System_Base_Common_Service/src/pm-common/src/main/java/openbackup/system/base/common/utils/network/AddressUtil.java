package openbackup.system.base.common.utils.network;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.utils.NumberUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.math.BigInteger;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.text.Normalizer;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Ip地址工具类
 *
 * @author s90004407
 * @version [Lego V100R002C10, 2014-12-18]
 * @since 2019-11-01
 */
public final class AddressUtil {
    /**
     * IPV4类型
     */
    public static final String IP4 = "IPV4";

    /**
     * IPV6类型
     */
    public static final String IP6 = "IPV6";

    /**
     * ipv6地址的校验正则表达式，目前和DeviceManager融合存储V5R7C50保持一致，DM对齐接口人 李晴川 l00244791
     * 此ipv6正则可以通过ipv6和ipv4的地址
     */
    public static final String IPV4REG =
            "((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]\\d|\\d)\\.){3}(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]\\d|\\d)";

    /**
     * IPV6REG
     */
    public static final String IPV6REG =
            "^(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|"
                    + "([0-9a-fA-F]{1,4}:){1,7}:|"
                    + "([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|"
                    + "([0-9a-fA-F]{1,4}:){1,5}:"
                    + IPV4REG
                    + "|"
                    + "([0-9a-fA-F]{1,4}:){6,6}"
                    + IPV4REG
                    + "|"
                    + "([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|"
                    + "([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|"
                    + "([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,1}:"
                    + IPV4REG
                    + "|"
                    + "([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|"
                    + "([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,2}:"
                    + IPV4REG
                    + "|"
                    + "([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|"
                    + "([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,3}:"
                    + IPV4REG
                    + "|"
                    + "[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|"
                    + "[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,4}):"
                    + IPV4REG
                    + "|"
                    + ":((:[0-9a-fA-F]{1,4}){1,5}):"
                    + IPV4REG
                    + "|"
                    + ":((:[0-9a-fA-F]{1,4}){1,7}|:)|"
                    + "[Ff][Ee]08:(:[0-9a-fA-F]{1,4}){2,2}%[0-9a-zA-Z]{1,}|"
                    + "(0{1,4}:){6,6}"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:){1,5}:"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:){1,4}:(0{1,4}:)"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:){1,3}:(0{1,4}:){1,2}"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:){1,2}:(0{1,4}:){1,3}"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:):(0{1,4}:){1,4}"
                    + IPV4REG
                    + "|"
                    + "::(0{1,4}:){1,5}"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:){5,5}"
                    + "[Ff]{4}:"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:){1,4}:"
                    + "[Ff]{4}:"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:){1,3}:(0{1,4}:)"
                    + "[Ff]{4}:"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:){1,2}:(0{1,4}:){1,2}"
                    + "[Ff]{4}:"
                    + IPV4REG
                    + "|"
                    + "(0{1,4}:):(0{1,4}:){1,3}"
                    + "[Ff]{4}:"
                    + IPV4REG
                    + "|"
                    + "::(0{1,4}:){1,4}"
                    + "[Ff]{4}:"
                    + IPV4REG
                    + "|"
                    + "::([Ff]{4}:){0,1}"
                    + IPV4REG
                    + ")$";

    private static final int VALUE_0X00FFFFFF = 0x00FFFFFF;

    private static final int VALUE_0X0000FFFF = 0x0000FFFF;

    private static final int VALUE_0X000000FF = 0x000000FF;

    private static final Pattern PATTERN_IPV6 = Pattern.compile(IPV6REG);

    private static final Pattern PATTERN_IPV4 = Pattern.compile("\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}");

    private static final Logger LOGGER = LoggerFactory.getLogger(AddressUtil.class);

    private AddressUtil() {}

    /**
     * 检查IP是否是本机IP
     *
     * @param ip ip
     * @return boolean
     */
    public static boolean checkLocalIP(String ip) {
        if (ip == null || ip.length() <= 0) {
            return false;
        }

        if ("127.0.0.1".equals(ip)) {
            return true;
        }

        if ("localhost".equalsIgnoreCase(ip)) {
            return true;
        }

        InetAddress[] ipsAddr = getAddress();

        String[] localServers = new String[ipsAddr.length];
        for (int i = 0; i < ipsAddr.length; i++) {
            if (ipsAddr[i] != null) {
                localServers[i] = ipsAddr[i].getHostAddress();
                if (ip.equals(localServers[i])) {
                    return true;
                }
            }
        }
        return false;
    }

    private static InetAddress[] getAddress() {
        InetAddress addrd;
        try {
            addrd = InetAddress.getLocalHost();
        } catch (UnknownHostException ee) {
            throw new EmeiStorDefaultExceptionHandler(ee.getMessage());
        }

        String hostName = addrd.getHostName();
        InetAddress[] ipsAddr = null;
        try {
            ipsAddr = InetAddress.getAllByName(hostName);
        } catch (UnknownHostException ex) {
            throw new EmeiStorDefaultExceptionHandler(ex.getMessage());
        }
        return ipsAddr;
    }

    /**
     * 得到本机ip
     *
     * @return String 本机ip
     */
    public static String getLocalIP() {
        InetAddress[] ipsAddr = getAddress();

        if (ipsAddr != null && ipsAddr.length > 0) {
            return ipsAddr[0].getHostAddress();
        }
        return "";
    }

    /**
     * IP地址转换为数字类型
     *
     * @param ipaddress ip地址
     * @return 数字类型，ipv4 32bit，ipv6 128bit
     */
    public static Optional<BigInteger> ipToInt(String ipaddress) {
        if (isIPv4Address(ipaddress)) {
            long longIp = Ipv4AddressUtil.ipv4ToLong(ipaddress);
            return Optional.of(BigInteger.valueOf(longIp));
        } else if (isIPv6Address(ipaddress)) {
            return Optional.of(ipv6ToInt(ipaddress));
        } else {
            LOGGER.error("this ip is not a valid ip address, ip: {}.", ipaddress);
            return Optional.empty();
        }
    }

    /**
     * ip地址 转换成 十进制整数形式
     *
     * @param ip ipaddress
     * @return long [返回类型说明]
     */
    public static BigInteger ipv6ToInt(String ip) {
        try {
            String ipaddress = ip;
            // ipv6地址格式校验,可能包含% "IPv6address%prefix"形式
            if (ipaddress.contains("%")) {
                ipaddress = ipaddress.split("%")[0];
            }
            int compressIndex = ipaddress.indexOf("::");
            if (compressIndex != LegoNumberConstant.NEGATIVE_ONE) {
                return handleIpv6HasDoubleColon(ipaddress, compressIndex);
            }
            return handleIpv6HasColon(ipaddress);
        } catch (Exception ex) {
            LOGGER.error("handle error", ex);
        }

        return BigInteger.ZERO;
    }

    /**
     * 处理ip地址中包含:的情况
     *
     * @param ipaddress ipv6地址
     * @return 十进制的地址
     */
    private static BigInteger handleIpv6HasColon(String ipaddress) {
        String[] str = ipaddress.split(":");
        BigInteger big = BigInteger.ZERO;
        for (int i = 0; i < str.length; i++) {
            // ::1
            if (str[i].isEmpty()) {
                str[i] = "0";
            }
            big =
                    big.add(
                            BigInteger.valueOf(Long.valueOf(str[i], IsmNumberConstant.SIXTTEEN))
                                    .shiftLeft(IsmNumberConstant.SIXTTEEN * (str.length - i - 1)));
        }
        return big;
    }

    /**
     * 处理ipv6地址中包含::的情况
     *
     * @param ipaddress ipv6地址
     * @param compressIndex ::符号在字符串中的位置
     * @return 十进制的地址
     */
    private static BigInteger handleIpv6HasDoubleColon(String ipaddress, int compressIndex) {
        String part1s = ipaddress.substring(0, compressIndex);
        String part2s = ipaddress.substring(compressIndex + 1);
        BigInteger part1 = ipv6ToInt(part1s);
        BigInteger part2 = ipv6ToInt(part2s);
        int part1hasDot = 0;
        char[] charArray = part1s.toCharArray();
        for (char value : charArray) {
            if (value == ':') {
                part1hasDot++;
            }
        }
        // ipv6 has most 7 dot
        return part1.shiftLeft(LegoNumberConstant.SIXTEEN * (LegoNumberConstant.SEVEN - part1hasDot)).add(part2);
    }

    /**
     * 将十进制整数形式转换成127.0.0.1形式的ip地址
     * 将整数值进行右移位操作（>>>），右移24位，右移时高位补0，得到的数字即为第一段IP。
     * 通过与操作符（&）将整数值的高8位设为0，再右移16位，得到的数字即为第二段IP。
     * 通过与操作符把整数值的高16位设为0，再右移8位，得到的数字即为第三段IP。
     * 通过与操作符把整数值的高24位设为0，得到的数字即为第四段IP。
     *
     * @param ipaddress ipaddress
     * @return String String
     */
    public static String longToIPv4(long ipaddress) {
        StringBuffer sb = new StringBuffer();
        // 直接右移24位
        sb.append(ipaddress >>> LegoNumberConstant.TWENTY_FOUR);
        sb.append('.');
        // 将高8位置0，然后右移16位
        sb.append((ipaddress & VALUE_0X00FFFFFF) >>> LegoNumberConstant.SIXTEEN);
        sb.append('.');
        // 将高16位置0，然后右移8位
        sb.append((ipaddress & VALUE_0X0000FFFF) >>> LegoNumberConstant.EIGHT);
        sb.append('.');
        // 将高24位置0
        sb.append(ipaddress & VALUE_0X000000FF);
        return sb.toString();
    }

    /**
     * 检查所给的是否为一个合法的IP地址 下边三种都是特殊地址，不能用于IP用户 0.* 127.* 255.255.255.255
     * 224.*以上的，都是组播或保留地址，也不能用于IP用户。
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
     * 检查所给的是否为一个合法的IP地址 下边三种都是特殊地址，
     * 组播或保留地址，也不能用于IP用户。
     *
     * @param ipv6 ip
     * @return true: 合法的IP地址 false: 非法的IP地址
     */
    public static boolean isValidIPv6(String ipv6) {
        if (isIPv6Address(ipv6)) {
            try {
                InetAddress inetAddress = InetAddress.getByName(ipv6);
                if (!(inetAddress instanceof Inet6Address)) {
                    return false;
                }
                Inet6Address address = (Inet6Address) inetAddress;
                return !address.isMulticastAddress() && !address.isLoopbackAddress() && !address.isAnyLocalAddress();
            } catch (UnknownHostException e) {
                LOGGER.error("check isValidIPV6 failed.", e);
                return false;
            } catch (Exception ex) {
                LOGGER.error("check isValidIPV6 error.", ex);
                return isValidIPv4(ipv6);
            }
        } else {
            return isValidIPv4(ipv6);
        }
    }

    /**
     * 检查所给的是否为一个合法的IP地址 下边三种都是特殊地址，
     * 组播或保留地址，也不能用于IP用户。
     *
     * @param ipv6 ip
     * @return true: 合法的IP地址 false: 非法的IP地址
     */
    public static boolean isValidIPv6Only(String ipv6) {
        if (isIPv6Address(ipv6)) {
            try {
                InetAddress inetAddress = InetAddress.getByName(ipv6);
                if (!(inetAddress instanceof Inet6Address)) {
                    return false;
                }
                Inet6Address address = (Inet6Address) inetAddress;
                return !address.isMulticastAddress() && !address.isLoopbackAddress() && !address.isAnyLocalAddress();
            } catch (UnknownHostException e) {
                LOGGER.error("check isValidIPV6 failed.", e);
                return false;
            } catch (Exception ex) {
                LOGGER.error("check isValidIPV6 error.", ex);
                return false;
            }
        }
        return false;
    }

    /**
     * 根据子网掩码的位数得到 *.*.*.* 格式的子网掩码
     *
     * @param mask mask
     * @return String [返回类型说明]
     */
    public static String maskToIp(int mask) {
        int varx = mask / LegoNumberConstant.EIGHT;
        int vary = mask % LegoNumberConstant.EIGHT;
        String part = "";
        for (int i = 0; i < varx; i++) {
            if (varx == LegoNumberConstant.FOUR) {
                part = "255.255.255.255";
                break;
            }
            part = part + "255.";
        }

        int sum = 0;
        int constant = LegoNumberConstant.TWO_HUNDRED_FIFTY_SIX;
        for (int i = 0; i < vary; i++) {
            constant = constant >> 1;
            sum = sum + constant;
        }

        StringBuffer buf = new StringBuffer();
        if (varx != LegoNumberConstant.FOUR) {
            for (int i = 0; i < (LegoNumberConstant.THREE - varx); i++) {
                buf = buf.append(".0");
            }
            part = part + sum + buf;
        }

        return part;
    }

    /**
     * ipv6参数检查
     *
     * @param ip ip...
     * @return 参数是否是IPV6地址
     */
    public static boolean isIPv6Address(String ip) {
        if (VerifyUtil.isEmpty(ip)) {
            LOGGER.debug("ip is null.");
            return false;
        }
        String ipv6 = Normalizer.normalize(ip, Normalizer.Form.NFKC);
        Matcher mat = PATTERN_IPV6.matcher(ipv6);
        if (mat.matches()) {
            try {
                InetAddress inetAddress = InetAddress.getByName(ipv6);
                return inetAddress instanceof Inet6Address;
            } catch (UnknownHostException ex1) {
                LOGGER.error(ex1.getMessage(), ex1);
                return false;
            } catch (Exception ex2) {
                LOGGER.debug(ex2.getMessage(), ex2);
                return false;
            }
        } else {
            LOGGER.debug("{} :is not a ipv6 address.", ipv6);
            return false;
        }
    }

    /**
     * ipv4参数检查
     *
     * @param ip ip...
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
                    LOGGER.debug("{} is not a ipv4 address.", ipv4);
                    return false;
                }
            } catch (UnknownHostException ex) {
                LOGGER.debug(ex.getMessage(), ex);
                return false;
            }
        } else {
            LOGGER.debug("{} is not a ipv4 address.", ipv4);
            return false;
        }
    }

    /**
     * ip地址合法性检查
     *
     * @param ip ip...
     * @return 参数是否是合法
     */
    public static boolean isIpAddress(String ip) {
        return isIPv6Address(ip) || isIPv4Address(ip);
    }

    /**
     * 判断两个ip地址是否同种版本
     *
     * @param ip1 IP1
     * @param ip2 IP2
     * @return 是否同个版本
     */
    public static boolean isSameVersion(String ip1, String ip2) {
        // 同为IPV4
        if (ip1.contains(".") && ip2.contains(".")) {
            return true;
        }
        // 同为IPV6
        return ip1.contains(":") && ip2.contains(":");
    }

    /**
     * ipv6参数检查
     *
     * @param inputIp       需要填充全IPv6信息的地址。
     * @param fillString 地址段之间用什么分隔。
     * @return 参数是否是IPV6地址
     */
    public static String fillIpv6(String inputIp, String fillString) {
        String ipv6 = inputIp.trim();
        if (ipv6.startsWith(":")) {
            ipv6 = "0000" + ipv6;
        }
        if (ipv6.endsWith(":")) {
            ipv6 += "0000";
        }
        StringBuilder newIpv6 = new StringBuilder();
        String[] segments = ipv6.split(":");
        // 处理第一位和最后一一位为空的IP  比如： ::12F  12:: 这种IP
        if (VerifyUtil.isEmpty(segments[0])) {
            segments[0] = "0000";
        }
        if (VerifyUtil.isEmpty(segments[segments.length - 1])) {
            segments[segments.length - 1] = "0000";
        }
        // 依次用0填充不够四位的地址段，以及简写的地址段。
        for (String segment : segments) {
            if (VerifyUtil.isEmpty(segment)) {
                for (int i = 0; i <= LegoNumberConstant.EIGHT - segments.length; i++) {
                    newIpv6.append("0000").append(fillString);
                }
                continue;
            }
            if (segment.length() == 1) {
                newIpv6.append("000");
                newIpv6.append(segment);
            } else if (segment.length() == LegoNumberConstant.TWO) {
                newIpv6.append("00");
                newIpv6.append(segment);
            } else if (segment.length() == LegoNumberConstant.THREE) {
                newIpv6.append("0");
                newIpv6.append(segment);
            } else {
                newIpv6.append(segment);
            }
            newIpv6.append(fillString);
        }
        // 去除最后一个填充字符。
        return newIpv6.substring(0, newIpv6.length() - fillString.length());
    }

    /**
     * 返回ip的种类名称, ipv4或者ipv6, 如果ip不合法, 则返回""
     *
     * @param ip ip地址字符串
     * @return 返回ip类型
     */
    public static String ipType(String ip) {
        if (AddressUtil.isIPv4Address(ip)) {
            return IP4;
        } else if (AddressUtil.isIPv6Address(ip)) {
            return IP6;
        } else {
            return "";
        }
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
     * 输入ipv6地址,返回一个格式化后的IPv6字符串, 使用[]包裹起来, 参数ipStr是一个ipv6的地址, 在使用之前需要注意进行判断
     *
     * @param ipStr ip字符串
     * @return 返回格式化后的IPv6字符串
     */
    public static String formatIPv6(String ipStr) {
        return "[" + ipStr.trim().replaceAll("^\\[|\\]$", "") + "]";
    }

    /**
     * 去除ipStr之中的%, ip&网关, 此处不做校验, 只是去除, 获取第一个数组字符串
     *
     * @param ipStr ip字符串
     * @return ipStrResult 返回ip字符串结果
     */
    public static String getStrWithoutPrecent(String ipStr) {
        if (ipStr.contains("%")) {
            return ipStr.split("\\%")[0];
        }
        return ipStr;
    }

    /**
     * 判断是否是有效的IP地址，包括iPv4和Ipv6<br/>
     * Valid if ip address is right
     *
     * @param ip iIP地址
     * @return 是否是有效的ipv4或者ipv6地址
     */
    public static boolean isValidIP(String ip) {
        return isValidIPv4(ip) || isValidIPv6(ip);
    }

    /**
     * 判断一个IP是否是回环地址
     *
     * @param ip ip
     * @return true: 是回环地址， false:不是回环地址
     */
    public static boolean isLoopbackAddress(String ip) {
        try {
            String[] ips = ip.split(",");
            for (String ipStr : ips) {
                String ipAddress = Normalizer.normalize(ipStr, Normalizer.Form.NFKC);
                InetAddress address = InetAddress.getByName(ipAddress);
                if (address.isLoopbackAddress()) {
                    return true;
                }
            }
        } catch (UnknownHostException e) {
            LOGGER.error("check isValidIPV6 failed.", e);
            return true;
        }
        return false;
    }
}
