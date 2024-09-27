/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.database.base.plugin.interceptor;

import openbackup.data.access.framework.copy.mng.provider.BaseCopyDeleteInterceptor;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * 测试副本删除拦截器类
 *
 * @author h30027154
 * @since 2022-06-16
 */
@Component
public class TestBaseCopyDeleteInterceptor extends BaseCopyDeleteInterceptor {
    @Override
    public boolean applicable(String object) {
        return true;
    }

    @Override
    public List<String> getAssociatedCopy(String copyId) {
        return Arrays.asList("1", "2");
    }

    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return true;
    }
}
