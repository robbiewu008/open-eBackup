/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.pack.node.model;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-01
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
