package openbackup.obs.plugin.common.constants;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import java.util.regex.Pattern;

/**
 * 环境常量
 *
 * @author w00607005
 * @since 2023-11-15
 */
public class EnvironmentConstant {
    /**
     * ak
     */
    public static final String KEY_AK = "ak";

    /**
     * sk
     */
    public static final String KEY_SK = "sk";

    /**
     * proxyEnable
     */
    public static final String KEY_PROXY_ENABLE = "proxyEnable";

    /**
     * proxyHostName
     */
    public static final String KEY_PROXY_HOST_NAME = "proxyHostName";

    /**
     * proxyUserName
     */
    public static final String KEY_PROXY_USER_NAME = "proxyUserName";

    /**
     * proxyUserPwd
     */
    public static final String KEY_PROXY_USER_PWD = "proxyUserPwd";

    /**
     * 代理开启传入后端的值为"1"
     */
    public static final String PROXY_ENABLE_VALUE = "1";

    /**
     * 不开启代理
     */
    public static final String PROXY_DISABLE_VALUE = "0";

    /**
     * certificate
     */
    public static final String KEY_CERTIFICATION = "certification";

    /**
     * 对象存储数量上限
     */
    public static final int MAX_OBJECT_STORAGE_NUM = 2000;

    /**
     * storageType
     */
    public static final String KEY_STORAGE_TYPE = "storageType";

    /**
     * useHttps
     */
    public static final String KEY_USE_HTTPS = "useHttps";

    /**
     * agents
     */
    public static final String KEY_AGENTS = "agents";

    /**
     * bucketList
     */
    public static final String KEY_BUCKETLIST = "bucketList";

    /**
     * bucket id,用于自定义排队规则
     */
    public static final String KEY_BUCKET_ID = "bucket_Id";

    /**
     * 对象存储、对象集合名称正则
     */
    public static final Pattern NAME_PATTERN = Pattern.compile(
        "^[\\u4e00-\\u9fa5a-zA-Z_]{1}[\\u4e00-\\u9fa5a-zA-Z_0-9-]{0,63}$");

    /**
     * 注册使用HTTPS变量值
     */
    public static final String USE_HTTPS_VALUE = "1";

    /**
     * endpoint键值
     */
    public static final String ENDPOINT = "endpoint";

    /**
     * 域名校验规则
     */
    public static final String DOMAIN = "((?:(?!-)[a-zA-Z0-9-]{1,63}(?<!-)\\.)+[a-zA-Z]{2,63})";

    /**
     * 域名:端口正则
     */
    public static final Pattern DOMAIN_PORT_PATTERN =
            Pattern.compile("^" + DOMAIN + "(:" + RegexpConstants.PORT + ")?$");

    /**
     * IP:PORT正则
     */
    public static final Pattern IP_PORT_PATTERN = Pattern.compile(RegexpConstants.IP_PORT);
}
