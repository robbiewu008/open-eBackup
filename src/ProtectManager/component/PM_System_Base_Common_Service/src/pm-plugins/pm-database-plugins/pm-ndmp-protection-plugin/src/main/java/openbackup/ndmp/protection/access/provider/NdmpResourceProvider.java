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
package openbackup.ndmp.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.util.ProtectedResourceUtil;
import openbackup.ndmp.protection.access.common.NdmpCommon;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * Ndmp资源类提供者
 *
 */
@Component
@Slf4j
public class NdmpResourceProvider implements ResourceProvider {
    /**
     * 联通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.NDMP_BACKUPSET.getType().equals(object.getSubType());
    }

    /**
     * Ndmp支持索引
     *
     * @return boolean 是否支持索引
     */
    @Override
    public boolean isSupportIndex() {
        return true;
    }

    /**
     * 设置path信息，否则复制的时候会报错
     *
     * @param resource 资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        resource.setPath(resource.getName());
        NdmpCommon.setNdmpNames(resource);
    }

    /**
     * 集群环境修改的时候会去检查连通性、实例和数据库资源属于自动扫描。更新资源时不再去检查
     *
     * @param resource 资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
    }

    /**
     * 不支持lanfree的应用实现ResourceProvider接口
     *
     * @return GaussDB资源是否更新主机信息配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();
        resourceFeature.setSupportedLanFree(false);
        return resourceFeature;
    }

    @Override
    public boolean supplyDependency(ProtectedResource resource) {
        return true;
    }


    @Override
    public void cleanUnmodifiableFieldsWhenUpdate(ProtectedResource resource) {
        String parentName = resource.getParentName();
        ProtectedResourceUtil.cleanUnmodifiableFields(resource);
        // 存储设备名称可以修改
        resource.setParentName(parentName);
    }
}