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
package openbackup.system.base.common.validator.constants;

import openbackup.system.base.common.utils.network.AddressUtil;

/**
 * 正则规格类
 *
 */
public final class RegexpConstants {
    /**
     * Email正则表达式
     * Email地址定义：local-part@domain
     * local-part部分允许出现：字母、数字、"!"、"#"、"$"、"%"、"&"、"'"、"*"、"+"、"-"、
     * "/"、"="、"?"、"^"、"_"、"`"、"{"、"|"、"}"、"~"、"."
     * domain部分只允许出现：字母、数字、"_"、"-"
     * 注意domain部分可能会跟有多个域名后缀，例如：warning@home.sammy.co.jp
     */
    public static final String EMAIL = "^[a-zA-Z0-9\\.\\-_\\!\\#\\$\\%\\&\\'\\*\\+\\/\\=\\?\\^\\`\\{\\|\\}\\~]+"
        + "@[a-zA-Z0-9\\-_]+(\\.[a-zA-Z0-9\\-_]+)+$";

    /**
     * 证书对象里的DN信息分隔正则表达式
     */
    public static final String REGEXP_DN_INFO = "[,][ ]";

    /**
     * IPV4地址正则表达式
     */
    public static final String IPADDRESS_V4 =
        "([1-9]|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}";

    /**
     * IP地址通用正则表达式，正则可以通过ipv6和ipv4的地址
     */
    public static final String IP_V4V6_ADDRESS = IPADDRESS_V4 + "|" + AddressUtil.IPV6REG;

    /**
     * 端口号正则校验表达式，0~65535
     */
    public static final String PORT =
        "([0-9]|[1-9]\\d{1,3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])";

    /**
     * IP:PORT正则校验
     */
    public static final String IP_PORT = "(" + IP_V4V6_ADDRESS + ")" + "(:" + PORT + ")?$";

    /**
     * 正整数正则表达式  >=0
     * regExp=^[1-9]\d*|0$
     */
    public static final String INTEGER_NEGATIVE = "^[1-9]\\d*|0$";

    /**
     * Double正则表达式
     * regExp=^-?([1-9]\d*\.\d*|0\.\d*[1-9]\d*|0?\.0+|0)$
     */
    public static final String DOUBLE = "^-?([1-9]\\d*\\.\\d*|0\\.\\d*[1-9]\\d*|0?\\.0+|0)$";

    /**
     * 匹配由数字、26个英文字母或者下划线组成的字符串
     * regExp=^\w+$
     */
    public static final String STR_ENG_NUM_DOWN = "^\\w+$";

    /**
     * 日期和时间格式(简单格式验证)
     * YYYY-MM-DD HH:MM:SS
     * regExp=^\d{4}\-\d{2}\-\d{2}\s{1}\d{2}:\d{2}:\d{2}$
     */
    public static final String DATE_TIME_FORMAT = "^\\d{4}\\-\\d{2}\\-\\d{2}\\s{1}\\d{2}:\\d{2}:\\d{2}$";

    /**
     * 时间验证(简单格式验证)
     * HH:MM:SS
     * regExp=^\d{2}:\d{2}:\d{2}$
     */
    public static final String TIME_FORMAT = "^([0]?\\d{1}|[2][0-3]{1}|[1]\\d{1})\\:[0-5]?[0-9]{1}\\:[0-5]?[0-9]{1}$";

    /**
     * 判断是否是整数
     */
    public static final String INTEGRAL = "^-?\\d+$";

    /**
     * 验证字符串长度32位
     */
    public static final int STRING_LENGTH_32 = 32;

    /**
     * 验证字符串长度64位
     */
    public static final int STRING_LENGTH_64 = 64;

    /**
     * 验证字符串长度255位
     */
    public static final int STRING_LENGTH_255 = 255;

    /**
     * 名称的正则表达式
     */
    public static final String NAME_STR = "^[a-zA-Z0-9_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-\\.]*$";

    /**
     * 描述的正则表达式 允许空字符串或者字母、数字、中文字符、下划线、短横线或点
     */
    public static final String DESC_STR = "^[\\u4e00-\\u9fa5\\w-\\.]*$";

    /**
     * 名称的正则表达式
     */
    public static final String NAME_STR_NOT_START_WITH_NUM = "^[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5\\w-\\.]*$";

    /**
     * 通用邮箱格式长度254位
     */
    public static final int EMAIL_MAX_LENGTH = 255;

    /**
     * UUID正则表达式
     */
    public static final String UUID = "^[0-9a-f]{8}(-[0-9a-f]{4}){3}-[0-9a-f]{12}$";

    /**
     * 排除转义字符!':;|`$<>&-()#?"\*.
     */
    public static final String SFTP_EXCLUDE_SPECIAL_CHARACTER = "^[^':\\?\\\\\"<>\\|\\*;`\\$&\\-\\(\\)#\\!]+$";

    /**
     * UUID无分隔符正则表达式
     */
    public static final String UUID_N0_SEPARATOR = "[0-9a-f]{32}";

    /**
     * SFTP上传路径，首字符不能为.且首尾字符不能为空格，不能包含特殊字符!':;|`$<>&-()#?"\*
     */
    public static final String SFTP_UPLOAD_PATH_RULE = "^[^':\\?\\\\\"<>\\|\\*;`\\$&\\-\\(\\)#\\!]+$";

    /**
     * 只能由字母、数字、中文字符、-、_（下划线）组成，可以为空字符串
     */
    public static final String FILE_NAME_REGEXP_PATTERN = "[\\u4e00-\\u9fa5_a-zA-Z0-9_-]*";

    /**
     * 只能由字母、数字、中文字符、-、.、_（下划线）组成，可以为空字符串
     */
    public static final String NET_PLANE_REGEXP_PATTERN = "[\\u4e00-\\u9fa5_a-zA-Z0-9_\\.-]*";

    /**
     * 只能由字母、数字、中文字符、-、_（下划线）组成，只能由字母、数字、中文字符开头
     */
    public static final String ENV_NAME_PATTERN = "^$|[a-zA-Z_\\u4e00-\\u9fa5]{1}[\\u4e00-\\u9fa5_a-zA-Z0-9_-]*$";

    /**
     * 分页查询插件中，orderBy的值必须以+或-开头，后续第一个字符只能为字母，然后再由字母、数字、下划线、句点组成
     */
    public static final String REGEXP_PAGE_QUERY_ORDER_BY = "^[+-][a-zA-Z][\\w.]*";

    /**
     * 支持文件系统的名称由数字，字母，"-"、"."、"_"组成
     */
    public static final String FILE_SYSTEM_NAME_REGEX = "^[a-zA-Z0-9_\\-\\.]+$";

    /**
     * macs签名算法
     * safe 安全算法， compatible 兼容性算法
     */
    public static final String MAC_SIG_ALGORITHM = "^safe|compatible$";

    /**
     * 小写字母
     */
    public static final String LOWERCASE_LETTERS = "[a-z]";

    /**
     * 大写字母
     */
    public static final String UPPERCASE_LETTERS = "[A-Z]";

    /**
     * 数字
     */
    public static final String NUM = "[0-9]";

    /**
     * 特殊字符，包含以下，专用场景，请勿修改
     * `~!@#$%^&*()+=|{}':;',[].<>/?~\
     * "[`~!@#$%^&*()+=|{}':;',\\[\\].<>/?~\\\\]"
     */
    public static final String SPECIAL_CHARACTERS = "[`~!@#$%^&*()+=|{}':;',\\[\\].<>/?~\\\\]";

    /**
     * 空格
     */
    public static final String WHITESPACE = "\\s";

    /**
     * 最少匹配种类数
     */
    public static final int MATCH_TIMES = 4;

    /**
     * 中文标点，包含
     * ？！“”￥‘’（），—。、：；《》【】…
     */
    public static final String CHINESE_CHARACTER = "\\uff1f\\uff01\\u201c\\u201d\\uffe5\\u2018\\u2019\\uff08\\uff09"
        + "\\uff0c\\u2014\\u3002\\u3001\\uff1a\\uff1b\\u300a\\u300b\\u3010\\u3011\\u2026";

    /**
     * 用于字符串长度校验时替换中文字符串的正则
     * 只能用来做中文以及字符串长度转换的校验，不能作为字符串内容校验
     * 匹配中文和中文标点符号，包含以下标点符号
     * ？！“”￥‘’（），—。、：；《》【】…
     */
    public static final String CHINESE_CHARACTER_LEN_VALID = "[\\u4e00-\\u9fa5" + CHINESE_CHARACTER + "]";

    /**
     * 用于匹配IPV4的子网掩码
     */
    public static final String IPV4_SUB_NETMASK =
        "(((0|128|192|224|240|248|252|254)\\.0\\.0\\.0)|(255\\.(((0|128|192|224|240|248|252|254)\\.0\\.0)|"
            + "(255\\.(((0|128|192|224|240|248|252|254)\\.0)|255\\.(0|128|192|224|240|248|252|254|255))))))";

    /**
     * 用于匹配IPV6的子网掩码
     */
    public static final String IPV6_SUB_NETMASK = "(0?\\d{1,2}|1([0-1]\\d|2[0-8]))";

    /**
     * 用于匹配IPV4或V6的子网掩码
     */
    public static final String IPV4V6_SUB_NETMASK = IPV4_SUB_NETMASK + "|" + IPV6_SUB_NETMASK;

    /**
     * IPV4 ip,允许0.0.0.0
     */
    public static final String IPADDRESS_V4_WITH_DEFAULT =
        "([0-9]|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])(\\.(\\d|[1-9]\\d|1\\d{2}|2[0-4]\\d|25[0-5])){3}";

    /**
     * IPV4/IPV6 ip,允许0.0.0.0
     */
    public static final String IP_V4V6_ADDRESS_WITH_DEFAULT = IPADDRESS_V4_WITH_DEFAULT + "|" + AddressUtil.IPV6REG;

    /**
     * 以http://或https://开头
     */
    public static final String WEB_URL = "^([hH]{1}[tT]{2}[pP]{1}[sS]?):\\/\\/[\\s\\S]*$";

    /**
     * 安装路径
     */
    public static final String INSTALL_PATH = "^[^|;&$><`!]*$";

    private RegexpConstants() {
    }
}
