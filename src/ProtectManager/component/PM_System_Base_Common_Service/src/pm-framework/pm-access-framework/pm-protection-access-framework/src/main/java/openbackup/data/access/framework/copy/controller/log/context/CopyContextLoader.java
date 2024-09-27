/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.controller.log.context;

import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import org.springframework.stereotype.Component;

/**
 * 副本上下文，用于操作日志记录
 *
 * @author: y00559272
 * @version: [OceanProtect X8000 1.2.1]
 * @since: 2022/8/17
 **/
@Component
@CalleeMethods(name = "copy_context_loader",
    value = {@CalleeMethod(name = "getById")})
public class CopyContextLoader {
    private final CopyRestApi copyRestApi;

    /**
     * 副本上下文加载构造函数
     *
     * @param copyRestApi 副本api
     */
    public CopyContextLoader(CopyRestApi copyRestApi) {
        this.copyRestApi = copyRestApi;
    }

    /**
     * 根据id查询副本信息
     *
     * @param copyId 副本id
     * @return 副本信息
     */
    public Copy getById(String copyId) {
        return copyRestApi.queryCopyByID(copyId, true);
    }
}
