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
package openbackup.system.base.pack.lock.zookeeper.zookeeper.impl;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.CommUtil;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.ZookeeperService;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.model.ZkNodeCacheListener;

import org.apache.curator.RetryPolicy;
import org.apache.curator.framework.CuratorFramework;
import org.apache.curator.framework.CuratorFrameworkFactory;
import org.apache.curator.framework.recipes.cache.NodeCache;
import org.apache.curator.framework.recipes.locks.InterProcessMutex;
import org.apache.curator.retry.ExponentialBackoffRetry;
import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.data.Stat;
import org.springframework.beans.factory.DisposableBean;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import java.nio.charset.StandardCharsets;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-05-30
 */
@Component
@Slf4j
public class ZookeeperServiceImpl implements ZookeeperService, InitializingBean, DisposableBean {
    private static final int RETRY_NUM = 3;

    @Value("${zookeeper.session.timeout}")
    private int sessionTimeout;

    @Value("${zookeeper.connection.timeout}")
    private int connectionTimeout;

    @Value("${zookeeper.client.url}")
    private String zkUrl;

    private CuratorFramework client;

    /**
     * 后处理初始化
     */
    @Override
    public void afterPropertiesSet() {
        // baseSleepTimeMs：初始的重试等待时间
        // maxRetries：最多重试次数
        RetryPolicy retryPolicy = new ExponentialBackoffRetry(connectionTimeout, RETRY_NUM);
        // 创建 CuratorFrameworkImpl实例
        client = CuratorFrameworkFactory.builder()
            .namespace("emeistor")
            .connectString(zkUrl)
            .sessionTimeoutMs(sessionTimeout)
            .connectionTimeoutMs(connectionTimeout)
            .retryPolicy(retryPolicy)
            .build();

        // 启动
        client.start();
        try {
            client.blockUntilConnected();
        } catch (InterruptedException e) {
            log.error("zookeeper curator block failed.", e);
        }
    }

    /**
     * bean销毁函数
     */
    @Override
    public void destroy() {
        CommUtil.closeQuietly(client);
    }

    /**
     * 获取节点的二进制值
     *
     * @param node 节点名字
     * @return 二进制值
     */
    @Override
    public byte[] getValue(String node) {
        try {
            Stat stat = client.checkExists().forPath(node);
            if (stat == null) {
                return null;
            } else {
                return client.getData().forPath(node);
            }
        } catch (Exception e) {
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * setValue
     *
     * @param node node
     * @param value 二进制值
     * @param createMode 节点类型
     */
    @Override
    public void setValue(String node, byte[] value, CreateMode createMode) {
        try {
            // 异步设置某个节点数据
            client.create().orSetData().creatingParentContainersIfNeeded().withMode(createMode).forPath(node, value);
        } catch (Exception e) {
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * deleteValue
     *
     * @param node 节点路径
     */
    @Override
    public void deleteValue(String node) {
        if (getValue(node) == null) {
            return;
        }
        try {
            List<String> children = client.getChildren().forPath(node);
            for (String child : children) {
                deleteValue(node + "/" + child);
            }
            client.delete().forPath(node);
        } catch (Exception e) {
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * getString
     *
     * @param node 节点名字
     * @return String
     */
    @Override
    public String getString(String node) {
        byte[] value = getValue(node);
        return value != null ? new String(value, StandardCharsets.UTF_8) : null;
    }

    /**
     * setString
     *
     * @param node node
     * @param value value
     * @param createMode 节点类型
     */
    @Override
    public void setString(String node, String value, CreateMode createMode) {
        setValue(node, value.getBytes(StandardCharsets.UTF_8), createMode);
    }

    /**
     * batchSetNodeValue
     *
     * @param nodeMap 批量map
     */
    @Override
    public void batchSetNodeValue(Map<String, String> nodeMap) {
        if (nodeMap == null || nodeMap.isEmpty()) {
            return;
        }

        try {
            for (Map.Entry<String, String> entry : nodeMap.entrySet()) {
                client.create()
                    .orSetData()
                    .creatingParentContainersIfNeeded()
                    .forPath(entry.getKey(), entry.getValue().getBytes(StandardCharsets.UTF_8));
            }
        } catch (Exception e) {
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * addNodeListener
     *
     * @param node 节点
     * @param listener 监听器
     */
    @Override
    public void addNodeListener(String node, ZkNodeCacheListener listener) {
        try {
            final NodeCache cache = new NodeCache(client, node, false);
            cache.start(true);
            listener.setCache(cache);
            cache.getListenable().addListener(listener);
        } catch (Exception e) {
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * createLock
     *
     * @param node 节点
     * @return InterProcessMutex
     */
    @Override
    public InterProcessMutex createLock(String node) {
        // 初始化分布式锁
        return new InterProcessMutex(client, node);
    }
}
