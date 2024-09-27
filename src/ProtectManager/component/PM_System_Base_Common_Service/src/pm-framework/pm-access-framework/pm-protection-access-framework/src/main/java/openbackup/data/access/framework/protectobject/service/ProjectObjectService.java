/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.protectobject.service;

import openbackup.data.access.framework.protectobject.model.ProtectionExecuteCheckReq;

/**
 * ProjectObjectService
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/11/13
 */
public interface ProjectObjectService {
    /**
     * 操作保护对象时的回调操作
     *
     * @param protectionExecuteCheckReq protectionExecuteCheckReq
     */
    void checkProtectObject(ProtectionExecuteCheckReq protectionExecuteCheckReq);

    /**
     * 校验该资源之前的备份副本所在的存储单元：1.等于当前SLA中选择的存储单元 2.被包含在在当前SLA选择的存储单元组中
     *
     * @param slaId slaId
     * @param resourceId resourceId
     */
    void checkExistCopiesLocationBeforeProtect(String slaId, String resourceId);

    /**
     * 校验该资源之前的备份副本所在的存储单元：1.等于当前SLA中选择的存储单元 2.被包含在在当前SLA选择的存储单元组中
     *
     * @param storageType storageType
     * @param storageId storageId
     * @param resourceId resourceId
     */
    void checkBeforeManualReplication(String storageType, String storageId, String resourceId);
}
