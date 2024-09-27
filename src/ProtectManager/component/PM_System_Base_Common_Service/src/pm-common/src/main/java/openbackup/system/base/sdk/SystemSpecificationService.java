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
package openbackup.system.base.sdk;

/**
 * System Specification Service
 *
 * @author l00272247
 * @since 2021-03-20
 */
public interface SystemSpecificationService {
    /**
     * 获取集群节点数量
     *
     * @return 集群节点数量
     */
    int getClusterNodeCount();

    /**
     * 获取单节点任务最大并发数
     *
     * @return 单节点任务最大并发数
     */
    int getSingleNodeJobMaximumConcurrency();

    /**
     * 获取单节点任务最大限额数
     *
     * @return 单节点任务最大限额数
     */
    int getSingleNodeJobMaximumLimit();
}
