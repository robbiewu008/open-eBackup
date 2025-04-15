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
package openbackup.data.access.framework.protection.service.archive;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.sdk.copy.model.Copy;

import java.util.List;

/**
 * This interface defines the restore providers that need to be implemented by DataMover
 *
 */
public interface ArchiveCopyProvider extends DataProtectionProvider<String> {
    /**
     * 日志归档需要查询副本的依赖链,框架提供默认实现:日志副本到最近一个全量之间的所有备份副本
     *
     * @param copies copies
     * @return CopyDependencyQueryResponse
     */
    List<CopyDependencyQueryResponse> queryCopyDependenceChain(List<Copy> copies);
}
