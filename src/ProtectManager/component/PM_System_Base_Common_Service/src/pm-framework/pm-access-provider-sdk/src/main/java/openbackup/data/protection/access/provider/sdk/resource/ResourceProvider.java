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
package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.exception.NotImplementedException;
import openbackup.data.protection.access.provider.sdk.resource.model.ExecuteScanRes;
import openbackup.data.protection.access.provider.sdk.util.ProtectedResourceUtil;

import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

/**
 * 受保护资源Provider，该类定义受保护资源的接口定义。不用的应用需要创建逻辑资源
 * 如HDBS备份的文件集，HBASE的BackupSet，注册NAS共享时时需要实现该接口
 *
 */
public interface ResourceProvider extends DataProtectionProvider<ProtectedResource> {
    /**
     * 对资源信息进行检查
     *
     * @param resource 资源
     */
    default void check(ProtectedResource resource) {
    }

    /**
     * 更新时对资源信息进行检查
     *
     * @param resource 资源
     */
    default void updateCheck(ProtectedResource resource) {
    }

    /**
     * 检查受保护资源，创建逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码<br/>
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     *
     * @param resource 受保护资源
     */
    void beforeCreate(ProtectedResource resource);

    /**
     * 检查受保护资源， 修改逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码<br/>
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     *
     * 提供的资源不包含dependency信息，如果应用需要补齐depen信息，请调用 “补充资源的dependency信息” 接口
     *
     * @param resource 受保护资源
     */
    void beforeUpdate(ProtectedResource resource);

    /**
     * 扫描受保护资源， 可选实现。根据受保护资源决定是否实现该接口，
     * 如受保护的资源需要在ProtectManager进行持久化，则需实现，如VMware虚拟化环境；
     * 如不需要将资源在ProtectManager中实现，则无须实现，比如HDFS的目录、文件，HBase的命名空间，表等。
     *
     * @param resource 受保护资源
     * @return 受保护资源的子资源列表
     */
    default List<ProtectedResource> scan(ProtectedResource resource) {
        throw new NotImplementedException("no need to scan");
    }

    /**
     * 扫描流程后的处理资源，主要为入库更新资源等操作
     * 复杂的入库流程，由插件自行实现。需要{@link ExecuteScanRes#isEndExecute()}为true,表示后续不会执行框架扫描逻辑
     *
     * @param resource 要扫描的资源
     * @param protectedResourceList 扫描上来的资源
     * @return {@link ExecuteScanRes}
     */
    default ExecuteScanRes afterScanHandleResource(ProtectedResource resource,
        List<ProtectedResource> protectedResourceList) {
        return ExecuteScanRes.defaultValue();
    }

    /**
     * 资源是否支持对副本创建索引，默认实现不支持索引
     *
     * @return 支持返回true，不支持返回false
     */
    default boolean isSupportIndex() {
        return false;
    }

    /**
     * 资源的特性，比如是否校验名称重复
     *
     * @return ResourceFeature
     */
    default ResourceFeature getResourceFeature() {
        return ResourceFeature.defaultValue();
    }

    /**
     * 资源删除前的处理
     *
     * @param resource ProtectedResource
     * @return ResourceDeleteContext
     */
    default ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        return ResourceDeleteContext.defaultValue();
    }

    /**
     * 对资源进行健康检查
     *
     * @param resource 资源
     */
    default void healthCheck(ProtectedResource resource) {}

    /**
     * 更新时不可变的值
     *
     * @param resource 资源
     */
    default void cleanUnmodifiableFieldsWhenUpdate(ProtectedResource resource) {
        ProtectedResourceUtil.cleanUnmodifiableFields(resource);
    }

    /**
     * 关联的要删除的资源，返回子资源和孙子资源，包括本身
     * 关联资源层数和数量较多时，走框架通用逻辑较慢；这时通过插件实现，走特定逻辑
     *
     * @param protectedResource 资源
     * @return 关联的要删除的资源。
     */
    default Set<String> queryRelationResourceToDelete(ProtectedResource protectedResource) {
        return new HashSet<>();
    }

    /**
     * 填充依赖项
     *
     * @param resource 资源
     * @return true，表示插件实现； false，会继续执行框架逻辑
     */
    default boolean supplyDependency(ProtectedResource resource) {
        return false;
    }

    /**
     * 查询应用配置
     *
     * @param script 脚本
     * @param hostUuids 主机id
     * @return 配置map
     */
    default Map<String, Object> queryAppConf(String script, String[] hostUuids) {
        return new HashMap<>();
    }
}
