/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.access.framework.resource.util;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.SpringBeanUtils;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 功能描述
 *
 * @author c00642388
 * @since 2024-06-05
 */
@Component
@Slf4j
@CalleeMethods(name = "scan_resource_log_utils", value = {
    @CalleeMethod(name = "parseResourceLogParam")
})
public class ScanResourceLogUtils {
    /**
     * 构建资源参数，防勒索修改Storage名称为Storage，其他部署形态不变
     *
     * @param resId 资源ID
     * @return copy 参数
     */
    public ProtectedResourcePo parseResourceLogParam(String resId) {
        ProtectedResourceMapper protectedResourceMapper = SpringBeanUtils.getBean(ProtectedResourceMapper.class);
        DeployTypeService deployTypeService = SpringBeanUtils.getBean(DeployTypeService.class);
        ProtectedResourcePo resourcePo = protectedResourceMapper.selectById(resId);
        if (deployTypeService.isHyperDetectDeployType()) {
            String name = resourcePo.getName();
            if ("CloudBackupStorage".equals(name)) {
                resourcePo.setName("Storage");
            }
        }
        return resourcePo;
    }
}
