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
package openbackup.system.base.util;

/**
 * OpServiceUtil
 *
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
