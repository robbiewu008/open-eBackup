/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.agent.dto;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.util.BeanTools;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Check App Req
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/2/22
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class CheckAppReq {
    private AppEnv appEnv;

    private Application application;

    /**
     * 根据受保护资源或环境信息构造CheckAppReq
     *
     * @param resource 受保护资源或环境
     * @return CheckAppReq
     */
    public static CheckAppReq buildFrom(ProtectedResource resource) {
        AppEnv tempAppEnv = BeanTools.copy(resource, AppEnv::new);
        Application tempApplication = BeanTools.copy(resource, Application::new);
        return new CheckAppReq(tempAppEnv, tempApplication);
    }
}