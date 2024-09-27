/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.copy;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.sdk.copy.model.Copy;

/**
 * 副本校验拦截器
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-20
 */
public interface CopyVerifyInterceptor extends DataProtectionProvider<String> {
    /**
     * 副本校验拦截方法
     *
     * @param task 副本校验任务
     * @return CopyVerifyTask
     */
    CopyVerifyTask interceptor(CopyVerifyTask task);

    /**
     * 副本校验复制副本不允许副本校验不支持的副本类型列表
     *
     * @param copy 副本对象
     */
    void checkIsSupportVerify(Copy copy);
}
