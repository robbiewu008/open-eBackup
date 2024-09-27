/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.contant;

/**
 * 描述
 *
 * @author lwx544155
 * @version [OceanStor DJ V100R003C00, 2020年03月05日]
 * @see [相关类/方法]
 * @since [产品/模块版本]
 */
public class ConfigConstant {
    /** 配置文件中api北向接口前缀 */
    public static final String API_PREFIX = "api.mgr.";

    /**
     * COOKIE_PATH cookie根路径
     */
    public static final String COOKIE_PATH = "/console/";

    /**
     * CONSOLE /console
     */
    public static final String CONSOLE = "/console/rest";

    /**
     * SEPARATE 路径
     */
    public static final String SEPARATE = "/";

    /** session存储token的标识 */
    public static final String SESSION_TOKEN = "session_token";

    /** token标识 */
    public static final String TOKEN = "X-Auth-Token";

    /**
     * hcs-token
     */
    public static final String HCS_AUTH_TOKEN = "HCS-X-AUTH-TOKEN";

    /**
     * HCS 标识
     */
    public static final String HCS_FLAG = "OP-HCS";

    /**
     * DME 标识
     */
    public static final String DME_FLAG = "OP-DME";

    /**
     * dme-token
     */
    public static final String DME_AUTH_TOKEN = "Dme-X-Auth-Token";

    /**
     * dme-token
     */
    public static final String DME_AZ = "Az-Id";

    /**
     * REQUEST_ID标识
     */
    public static final String REQUEST_ID = "x-request-id";

    /**
     * 远端请求IP
     */
    public static final String REQUEST_IP = "x-forwarded-for";

    /**
     * CONTENT_DISPOSITION标识
     */
    public static final String CONTENT_DISPOSITION = "content-disposition";

    /**
     * CONTENT_TYPE标识
     */
    public static final String CONTENT_TYPE = "content-type";

    /**
     * session标识
     */
    public static final String SESSION = "EmeiSessionId";

    /**
     * CAPCHA标识
     */
    public static final String CAPCHA = "needVerificationCode";

    /**
     * SHOULD_VA标识
     */
    public static final int SHOULD_VA = 3;

    /**
     * 验证码KEY值
     */
    public static final String CAPTCHA_CODE = "captcha_code";

    /**
     * CSRF Token的cookie的key
     */
    public static final String CSRF_COOKIE_NAME = "_OP_TOKEN_";

    /**
     * opToken的userId的key
     */
    public static final String USER_ID = "userId";

    /**
     * CSRF前端携带的header名
     */
    public static final String HEADER_NAME = "opToken";

    /**
     * CSRF的长度
     */
    public static final int CSRF_TOKEN_LENGTH = 32;

    /**
     * cookie的同站策略sameSite值
     */
    public static final String SAME_SITE_STRATEGY = "Lax";

    /**
     * 集群类型
     */
    public static final String CLUSTER_TYPE = "clusters-type";

    /**
     * ip端口信息
     */
    public static final String HOST = "host";

    /**
     * ip端口信息
     */
    public static final String MANAGE_IP = "manage-ip";

    /**
     * 集群id
     */
    public static final String CLUSTER_ID = "clusters-id";

    /**
     * 本地集群类型
     */
    public static final String LOCAL_CLUSTER_TYPE = "1";

    /**
     * 添加了console配置后的根路径
     */
    public static final String CONSOLE_PATH = "/console/";

    /**
     * 成员节点ESN
     */
    public static final String MEMBER_ESN = "Member-esn";
}
