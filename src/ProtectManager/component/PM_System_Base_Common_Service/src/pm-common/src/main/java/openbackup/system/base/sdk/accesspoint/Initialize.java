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
package openbackup.system.base.sdk.accesspoint;

import openbackup.system.base.sdk.accesspoint.model.InitializeParam;
import openbackup.system.base.sdk.accesspoint.model.InitializeResult;
import openbackup.system.base.sdk.accesspoint.model.StandardBackupVolInitInfo;

import java.util.List;

/**
 * 初始化
 *
 */
public interface Initialize {
    /**
     * 初始化备份存储库
     *
     * @param initializeParam 初始化参数
     * @return 初始化结果
     */
    InitializeResult initializeBackStorage(InitializeParam initializeParam);

    /**
     * 获取当前节点挂载的卷信息
     *
     * @return 卷信息列表
     */
    List<StandardBackupVolInitInfo> queryVolumeInfo();
}
