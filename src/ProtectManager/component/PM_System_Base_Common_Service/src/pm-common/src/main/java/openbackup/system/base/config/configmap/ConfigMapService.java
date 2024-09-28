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
package openbackup.system.base.config.configmap;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.request.ClusterComponentPwdInfoRequest;
import openbackup.system.base.sdk.infrastructure.model.InfrastructureResponse;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.Iterator;

/**
 * 数据库信息获取
 *
 */
@FeignClient(name = "configMapService", url = "${service.url.infra}", configuration = CommonFeignConfiguration.class)
public interface ConfigMapService {
    /**
     * 获取名为common-conf的ConfigMap的配置项
     *
     * @return ConfigMapRes 配置信息
     */
    @GetMapping("/v1/infra/configmap/info?nameSpace=dpa&configMap=common-conf")
    @ResponseBody
    ConfigMapRes getCommonConfigMapData();

    /**
     * 创建configMap
     *
     * @param configMapReq 创建configMap请求
     */
    @PostMapping("infra/configmap/create")
    @ResponseBody
    void setCommonConfigMapData(@RequestBody ConfigMapReq configMapReq);

    /**
     * 获取名为common-secret的SecretMap配置项
     *
     * @return ConfigMapRes 配置信息
     */
    @GetMapping("/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret")
    @ResponseBody
    ConfigMapRes getCommonSecreteMapData();

    /**
     * 替换内部组件database/kafka/redis/cifs/dataturbo密码
     *
     * @param clusterComponentPwdInfoRequest 内部组件key和password值
     * @return InfrastructureResponse
     */
    @PostMapping("/v1/infra/external/cluster/update/password")
    InfrastructureResponse
        replaceComponentPassword(@RequestBody ClusterComponentPwdInfoRequest clusterComponentPwdInfoRequest);

    /**
     * 更新内部组件database/kafka/redis/cifs/dataturbo密码
     *
     * @return InfrastructureResponse
     */
    @PostMapping("/v1/infra/external/service/update/password")
    InfrastructureResponse updateComponentPassword();

    /**
     * 根据指定key获取ConfigMap的value值
     *
     * @param keyStr key
     * @return value
     */
    default String getValueFromConfigMapByKey(String keyStr) {
        String returnValue = "";
        ConfigMapRes dataSource = getCommonConfigMapData();
        JSONArray jsonArray = dataSource.getData();

        for (Iterator it = jsonArray.iterator(); it.hasNext();) {
            final Object next = it.next();
            if (next instanceof JSONObject) {
                JSONObject object = (JSONObject) next;
                if (object.containsKey(keyStr)) {
                    returnValue = object.getString(keyStr);
                    break;
                }
            }
        }
        return returnValue.trim();
    }

    /**
     * 获取vip连接地址
     *
     * @return vip连接地址
     */
    default String getVipAddress() {
        return getValueFromConfigMapByKey("vip.address");
    }

    /**
     * 获取任务过期时间
     *
     * @return 任务过期时间
     */
    default String getJobExpireTimeStr() {
        return getValueFromConfigMapByKey("job.Expire");
    }

    /**
     * 获取任务强制删除配置
     *
     * @return 任务强制删除配置
     */
    default String getDeleteJobForce() {
        return getValueFromConfigMapByKey("job.Delete");
    }

    /**
     * 获取部署类型配置
     *
     * @return 部署类型
     */
    default String getDeployType() {
        return getValueFromConfigMapByKey("deploy_type");
    }

    /**
     * 获取系统是否处于升级中
     *
     * @return 是否处于升级中
     */
    default boolean isSystemUpgrading() {
        String upgrading = getValueFromConfigMapByKey("upgrading");
        return Boolean.parseBoolean(upgrading);
    }
}
