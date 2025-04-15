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
package openbackup.data.protection.access.provider.sdk.protection.service;

import openbackup.data.protection.access.provider.sdk.protection.model.ProtectionExecuteDto;

import java.util.List;
import java.util.function.Consumer;

/**
 * 保护服务接口
 *
 */
public interface ProjectedObjectService {
    /**
     * 创建资源组保护 异步
     *
     * @param dto 请求体
     * @param postAction 后置处理
     * @return 任务id
     */
    String createGroupProjectedObject(ProtectionExecuteDto dto, Consumer<String> postAction);

    /**
     * 资源组修改后变更保护的子资源异步(新加的组中的虚拟机添加保护，移除出该组的移除保护)
     *
     * @param dto 请求体
     * @param toDeleteResourceIds 待移除的资源ids
     * @return 任务id
     */
    String changeGroupProjectedObject(ProtectionExecuteDto dto, List<String> toDeleteResourceIds);
}
