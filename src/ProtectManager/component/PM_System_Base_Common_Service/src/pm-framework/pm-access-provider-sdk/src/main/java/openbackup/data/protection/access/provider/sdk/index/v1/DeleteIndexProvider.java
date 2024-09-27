/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.index.v1;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 索引删除Provider(v1版本)
 *
 * @author g30003063
 * @since 2022-01-24
 */
public interface DeleteIndexProvider extends DataProtectionProvider<String> {
    /**
     * 删除索引
     *
     * @param requestId 请求ID
     * @param copyId 副本ID
     */
    void deleteIndex(String requestId, String copyId);
}