/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.constant;

import java.util.regex.Pattern;

/**
 * 功能描述
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-03
 */
public class KeyStoneConstant {
    /**
     * 注册的Service Name
     */
    public static final String SERVICE_NAME = "backupv2";

    /**
     * 注册的Service type
     */
    public static final String SERVICE_TYPE = "backupv2";

    /**
     * 请求的Token头参数key
     */
    public static final String TOKEN_HEADER = "X-Auth-Token";

    /**
     * 响应的Token头参数key
     */
    public static final String X_SUBJECT_TOKEN_HEADER = "X-Subject-Token";

    /**
     * 响应中的endpoints key
     */
    public static final String RESP_ENDPOINTS = "endpoints";

    /**
     * 响应中的regions key
     */
    public static final String RESP_REGIONS = "regions";

    /**
     * 响应中的services key
     */
    public static final String RESP_SERVICES = "services";

    /**
     * 响应中的service key
     */
    public static final String RESP_SERVICE = "service";

    /**
     * 响应中的URL key
     */
    public static final String RESP_URL = "url";

    /**
     * 响应中的ID key
     */
    public static final String RESP_ID = "id";

    /**
     * 注册的端口号
     */
    public static final String REGISTER_PORT = "25081";

    /**
     * endpoint的接口类型
     */
    public static final String ENDPOINT_INTERFACE = "public";

    /**
     * 请求keystone时的ssl协议版本
     */
    public static final String SSL_VERSION = "TLSv1.2";

    /**
     * 代理主机ip，云核场景注册key
     */
    public static final String CPS_IP = "cps_ip";

    /**
     * host请求头
     */
    public static final String HOST = "Host";

    /**
     * 请求name key
     */
    public static final String REQUEST_NAME = "name";

    /**
     * endpoint中获取域名的pattern
     */
    public static final Pattern DOMAIN_PATTERN = Pattern.compile("://(.*?):");
}
