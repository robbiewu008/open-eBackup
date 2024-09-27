/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.protection;

import java.util.Map;

/**
 * 操作受保护对形象
 *
 * @author y30044273
 * @version [OceanProtect X8000 1.5.0]
 * @since 2023-08-07
 */
public interface ProtectionOperateService {
    /**
     * 更新受保护对象的扩展字段
     *
     * @param updateMap 修改的扩展字段
     * @param resourceId 资源id
     */
    void updateProtectedObjectExtendParam(Map<String, Object> updateMap, String resourceId);
}
