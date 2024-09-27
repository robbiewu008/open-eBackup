/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.user;

/**
 * 功能描述
 *
 * @author z00842230
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-11
 */
public interface DomainResourceSetServiceApi {
    /**
     * 判断是否存在域-资源集关联关系
     *
     * @param domainId 域id
     * @param resourceObjectId 资源id
     * @param type 资源类型
     * @return 是否存在
     */
    String getResourceSetType(String domainId, String resourceObjectId, String type);
}
