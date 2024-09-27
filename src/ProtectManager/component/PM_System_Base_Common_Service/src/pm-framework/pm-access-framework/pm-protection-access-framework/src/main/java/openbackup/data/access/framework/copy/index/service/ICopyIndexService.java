/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.copy.index.service;

import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.index.v2.CopyIndexTask;

/**
 * 副本索引服务
 *
 * @author lWX776769
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021/12/31
 */
public interface ICopyIndexService {
    /**
     * 创建索引任务
     *
     * @param copyBo 副本对象
     * @param requestId 请求id
     * @param indexedMode 建索引方式
     * @return 副本索引任务对象
     */
    CopyIndexTask createIndexTask(CopyBo copyBo, String requestId, String indexedMode);

    /**
     * 删除资源索引任务
     *
     * @param resourceId 资源Id
     * @param userId 用户id
     */
    void deleteResourceIndexTask(String resourceId, String userId);

    /**
     * 删除副本索引任务
     *
     * @param requestId 请求id
     * @param copyId 副本id
     */
    void deleteCopyIndex(String requestId, String copyId);

    /**
     * 转发
     *
     * @param esn esn
     * @param copyId copyID
     */
    void forwardCreateIndex(String esn, String copyId);
}
