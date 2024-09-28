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
package openbackup.system.base.pack.node.model;

/**
 * 功能描述
 *
 */
public interface NodeListener {
    /**
     * 节点被删除回调
     *
     * @param node node
     */
    void nodeRemoved(String node);

    /**
     * 节点数据变化回调
     *
     * @param node  节点
     * @param value 值
     */
    void nodeChanged(String node, String value);
}
