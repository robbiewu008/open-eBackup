/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.pack.node;

import openbackup.system.base.pack.constant.NodeMode;
import openbackup.system.base.pack.node.model.NodeListener;

import java.io.File;
import java.util.Map;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-01
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
