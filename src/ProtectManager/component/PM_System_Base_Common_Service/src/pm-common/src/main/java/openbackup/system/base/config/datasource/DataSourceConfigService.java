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
package openbackup.system.base.config.datasource;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.Iterator;

/**
 * 数据库信息获取
 *
 * @author y30000858
 * @since 2021-01-22
 */
@FeignClient(name = "dataSourceService", url = "${service.url.infra}", configuration = CommonFeignConfiguration.class)
public interface DataSourceConfigService {
    /**
     * 获取名为common-conf的ConfigMap的配置项
     *
     * @return DataSourceRes 配置信息
     */
    @GetMapping("/v1/infra/configmap/info?nameSpace=dpa&configMap=common-conf")
    @ResponseBody
    DataSourceRes getDataSourceConfig();

    /**
     * 获取名为common-secret的SecretMap配置项
     *
     * @return DataSourceRes 配置信息
     */
    @GetMapping("/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret")
    @ResponseBody
    DataSourceRes getDataSourceConfigSecreteMap();

    /**
     * 获取vip连接地址
     *
     * @return vip连接地址
     */
    default String getVipAddress() {
        String vipAddress = "";
        DataSourceRes dataSource = getDataSourceConfig();
        JSONArray jsonArray = dataSource.getData();
        for (Iterator it = jsonArray.iterator(); it.hasNext();) {
            final Object next = it.next();
            if (!(next instanceof JSONObject)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "next is not instance of JSONObject");
            }
            JSONObject object = (JSONObject) next;
            if (object.containsKey("vip.address")) {
                vipAddress = object.getString("vip.address");
                break;
            }
        }
        return vipAddress.trim();
    }
}
