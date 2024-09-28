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
package openbackup.system.base.pack.node;

import openbackup.system.base.pack.constant.NodeMode;
import openbackup.system.base.pack.node.model.NodeListener;

import java.io.File;
import java.util.Map;

/**
 * 功能描述
 *
 */
public interface NodeService {
    /**
     * 获取节点的值
     *
     * @param node 节点名字
     * @return 值
     */
    byte[] getValue(String node);

    /**
     * 创建节点
     *
     * @param node     节点路径
     * @param value    节点值
     * @param nodeMode 节点类型
     */
    void setValue(String node, byte[] value, NodeMode nodeMode);

    /**
     * 创建节点,nodeMode默认使用持久化类型
     *
     * @param node  节点路径
     * @param value 节点值
     */
    default void setValue(String node, byte[] value) {
        setValue(node, value, NodeMode.PERSISTENT);
    }

    /**
     * 使用文件创建节点
     *
     * @param node     节点路径
     * @param file     文件
     * @param nodeMode 节点类型
     */
    void setValue(String node, File file, NodeMode nodeMode);

    /**
     * 使用文件创建节点,nodeMode默认使用持久化类型
     *
     * @param node 节点路径
     * @param file 文件
     */
    default void setValue(String node, File file) {
        setValue(node, file, NodeMode.PERSISTENT);
    }

    /**
     * 删除节点值，如果节点值不存在，则不做任何操作
     *
     * @param node 节点路径
     */
    void deleteValue(String node);

    /**
     * 获取节点的值
     *
     * @param node 节点名字
     * @return 值
     */
    String getString(String node);

    /**
     * 批量设置节点值
     *
     * @param nodeMap 批量map
     */
    void batchSetNodeValue(Map<String, String> nodeMap);

    /**
     * 创建节点
     *
     * @param node     节点路径
     * @param value    节点值
     * @param nodeMode 节点类型
     */
    void setString(String node, String value, NodeMode nodeMode);

    /**
     * 添加节点监听器
     *
     * @param node         节点
     * @param nodeListener 监听器
     */
    void addNodeListener(String node, NodeListener nodeListener);
}
