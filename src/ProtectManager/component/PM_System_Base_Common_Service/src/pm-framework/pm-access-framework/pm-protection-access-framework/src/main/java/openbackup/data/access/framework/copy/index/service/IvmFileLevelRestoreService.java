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

import openbackup.data.protection.access.provider.sdk.job.Task;
import openbackup.data.protection.access.provider.sdk.restore.RestoreObject;
import openbackup.system.base.security.exterattack.ExterAttack;

/**
 * 虚拟化应用细粒度恢复服务接口
 *
 */
public interface IvmFileLevelRestoreService {
    /**
     * 文件细粒度恢复
     *
     * @param restoreObject 恢复对象
     * @param snapMetaData 快照元数据
     * @return 任务信息
     */
    @ExterAttack
    Task fileLevelRestore(RestoreObject restoreObject, String snapMetaData);

    /**
     * 文件下载
     *
     * @param restoreObject 恢复对象
     * @param snapMetaData 快照元数据
     * @return 任务信息
     */
    @ExterAttack
    Task download(RestoreObject restoreObject, String snapMetaData);
}
