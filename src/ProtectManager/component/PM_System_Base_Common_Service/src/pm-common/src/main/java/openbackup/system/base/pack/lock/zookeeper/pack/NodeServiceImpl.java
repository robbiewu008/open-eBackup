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
package openbackup.system.base.pack.lock.zookeeper.pack;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.pack.constant.NodeMode;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.ZookeeperService;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.model.ZkNodeCacheListener;
import openbackup.system.base.pack.node.NodeService;
import openbackup.system.base.pack.node.model.NodeListener;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.apache.zookeeper.CreateMode;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Map;

/**
 * 功能描述
 *
 */
@Component
@Slf4j
public class NodeServiceImpl implements NodeService {
    @Autowired
    private ZookeeperService zookeeperService;

    @ExterAttack
    @Override
    public byte[] getValue(String node) {
        return zookeeperService.getValue(node);
    }

    @Override
    public void setValue(String node, byte[] value, NodeMode nodeMode) {
        zookeeperService.setValue(node, value, castAsCreateMode(nodeMode));
    }

    /**
     * setValue
     *
     * @param node 节点路径
     * @param file 文件
     * @param nodeMode 节点类型
     */
    @ExterAttack
    @Override
    public void setValue(String node, File file, NodeMode nodeMode) {
        try (InputStream is = FileUtils.openInputStream(file)) {
            int available = is.available();
            byte[] data = new byte[available];
            int count = IOUtils.read(is, data);
            if (count != available) {
                throw new LegoCheckedException("Inconsistent data");
            }
            setValue(node, data, nodeMode);
        } catch (FileNotFoundException e) {
            throw new LegoCheckedException("file missing: " + file.getName());
        } catch (IOException e) {
            log.error("load data failed", e);
        }
    }

    /**
     * deleteValue
     *
     * @param node 节点路径
     */
    @Override
    public void deleteValue(String node) {
        zookeeperService.deleteValue(node);
    }

    @ExterAttack
    @Override
    public String getString(String node) {
        return zookeeperService.getString(node);
    }

    @Override
    public void batchSetNodeValue(Map<String, String> nodeMap) {
        zookeeperService.batchSetNodeValue(nodeMap);
    }

    @Override
    public void setString(String node, String value, NodeMode nodeMode) {
        zookeeperService.setString(node, value, castAsCreateMode(nodeMode));
    }

    private CreateMode castAsCreateMode(NodeMode nodeMode) {
        if (nodeMode == null) {
            return null;
        }
        return CreateMode.valueOf(nodeMode.name());
    }

    @Override
    public void addNodeListener(String node, NodeListener nodeListener) {
        zookeeperService.addNodeListener(node, new ZkNodeCacheListener() {
            @Override
            public void nodeChanged() {
                if (this.getCache().getCurrentData() == null) {
                    // 节点被删除
                    nodeListener.nodeRemoved(this.getCache().getPath());
                } else {
                    // 节点数据改变
                    nodeListener.nodeChanged(this.getCache().getCurrentData().getPath(),
                            new String(this.getCache().getCurrentData().getData()));
                }
            }
        });
    }
}
