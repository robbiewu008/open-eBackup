/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.framework.copy.index.service;

import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.index.v2.CopyIndexTask;

/**
 * 副本索引服务
 *
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
