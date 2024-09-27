/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.util;

/**
 * OpServiceUtil
 *
 * @author l30044826
 * @since 2023-08-23
 */
public class OpServiceUtil {
    /**
     * 是否服务化
     *
     * @return 是否服务化
     */
    public static boolean isHcsService() {
        String isHcsService = System.getenv("GLOBAL_DOMAIN_NAME");
        return !org.apache.commons.lang3.StringUtils.isEmpty(isHcsService);
    }

    /**
     * 获取域名
     *
     * @return 域名
     */
    public static String getGlobalDomainName() {
        return System.getenv("GLOBAL_DOMAIN_NAME");
    }

    /**
     * 获取EXTERNAL_GLOBAL_DOMAIN_NAME域名
     *
     * @return 域名
     */
    public static String getExternaLGlobalDomainName() {
        return System.getenv("EXTERNAL_GLOBAL_DOMAIN_NAME");
    }

    /**
     * 获取region
     *
     * @return regionId
     */
    public static String getRegionId() {
        return System.getenv("REGION_ID");
    }
}
