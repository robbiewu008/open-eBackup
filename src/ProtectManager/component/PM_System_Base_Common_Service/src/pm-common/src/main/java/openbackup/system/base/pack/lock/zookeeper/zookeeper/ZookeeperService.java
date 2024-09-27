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
package openbackup.system.base.pack.lock.zookeeper.zookeeper;

import openbackup.system.base.pack.lock.zookeeper.zookeeper.model.ZkNodeCacheListener;

import org.apache.curator.framework.recipes.locks.InterProcessMutex;
import org.apache.zookeeper.CreateMode;

import java.util.Map;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-05-30
 */
public interface ZookeeperService {
    /**
     * 获取节点的二进制值
     *
     * @param node 节点名字
     * @return 二进制值
     */
    byte[] getValue(String node);

    /**
     * 设置节点二进制值
     *
     * @param node node
     * @param value 二进制值
     * @param createMode 节点类型
     */
    void setValue(String node, byte[] value, CreateMode createMode);

    /**
     * 删除节点值，如果节点值不存在，则不做任何操作
     *
     * @param node 节点路径
     */
    void deleteValue(String node);

    /**
     * 获取节点的字符串值
     *
     * @param node 节点名字
     * @return 字符串值
     */
    String getString(String node);

    /**
     * 设置节点值
     *
     * @param node node
     * @param value value
     * @param createMode 节点类型
     */
    void setString(String node, String value, CreateMode createMode);

    /**
     * 批量设置节点值
     *
     * @param nodeMap 批量map
     */
    void batchSetNodeValue(Map<String, String> nodeMap);

    /**
     * 监听某个节点
     *
     * @param node 节点
     * @param listener 监听器
     */
    void addNodeListener(String node, ZkNodeCacheListener listener);

    /**
     * 创建分布式锁
     *
     * @param node 节点
     * @return 分布式锁
     */
    InterProcessMutex createLock(String node);
}
