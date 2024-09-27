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
package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.config.business.initialize.StorageVolumeConfig;

import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 初始化备份存储参数
 *
 * @author w00493811
 * @since 2020-12-26
 */
@Data
@Slf4j
@NoArgsConstructor
@JsonInclude(JsonInclude.Include.NON_NULL)
public class InitializeParam {
    private static final int PATH_NUMBER = 4;

    private Map<String, InitializeParamNodeInfo> nodeNameToNodeParmInfos;

    private boolean isMountPathOneNode;

    /**
     * 默认构造函数
     *
     * @param initializeParamNodeInfos NAS源地址
     * @param volumeNameMap NAS共享路径与类型的对应表
     */
    public InitializeParam(List<InitializeParamNodeInfo> initializeParamNodeInfos,
        Map<Integer, String> volumeNameMap) {
        // 将初始化参数节点信息列表排序
        initializeParamNodeInfos.sort(InitializeParamNodeInfo.getComparator());

        // 遍历initializeParamPaths中的每个节点配置信息
        for (int index = 0; index < initializeParamNodeInfos.size(); index++) {
            // 批量遍历源路径列表，进行分配
            InitializeParamNodeInfo nodeInfo = initializeParamNodeInfos.get(index);
            nodeInfo.setNasPathForSelfBackVolume(volumeNameMap.get(StorageVolumeConfig.VOLUME_TYPE_SELF_BACKUP));
            nodeInfo.setNasPathForCloudIdxVolume(volumeNameMap.get(StorageVolumeConfig.VOLUME_TYPE_COULD_BACKUP));
            nodeInfo.setNasPathForMetaDataVolume(volumeNameMap.get(StorageVolumeConfig.VOLUME_TYPE_METADATA_BACKUP));
            nodeInfo.setNasPathForStandardVolume(volumeNameMap.get(StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP));
            nodeInfo.setNasSourcePath(volumeNameMap.get(StorageVolumeConfig.VOLUME_TYPE_FILE_SYSTEM));
        }

        nodeNameToNodeParmInfos = new HashMap<>();
        initializeParamNodeInfos.forEach(info -> nodeNameToNodeParmInfos.put(info.getPodName(), info));
    }

    /**
     * 重置参数配置
     *
     * @param initializeParamNodeInfos 初始化节点信息
     * @param sourcePathList NAS共享源路径列表，此处theSourcePathList必须是有序的
     */
    public void reset(List<InitializeParamNodeInfo> initializeParamNodeInfos, List<String> sourcePathList) {
        // 将源路径列表排序
        List<String> theSourcePathList = preDealPathList(sourcePathList);

        // 收集已经配置的节点信息
        List<InitializeParamNodeInfo> needDelete = new ArrayList<>();
        for (InitializeParamNodeInfo test : initializeParamNodeInfos) {
            // 已经配置的数据不需再次处理，剩下的数据需要重新分配
            if (nodeNameToNodeParmInfos.containsKey(test.getPodName())) {
                // 待删除
                needDelete.add(test);

                InitializeParamNodeInfo initializeParamNodeInfo = nodeNameToNodeParmInfos.get(test.getPodName());
                // 删除已经配置的路径信息
                theSourcePathList.remove(initializeParamNodeInfo.getNasPathForStandardVolume());
                theSourcePathList.remove(initializeParamNodeInfo.getNasPathForMetaDataVolume());
                theSourcePathList.remove(initializeParamNodeInfo.getNasPathForCloudIdxVolume());
                theSourcePathList.remove(initializeParamNodeInfo.getNasPathForSelfBackVolume());
            }
        }

        // 删除已经配置的节点信息
        initializeParamNodeInfos.removeAll(needDelete);

        // 将初始化参数节点信息列表排序
        initializeParamNodeInfos.sort(InitializeParamNodeInfo.getComparator());

        // 将源路径列表排序
        theSourcePathList.sort(null);

        // 遍历initializeParamPaths中的每个节点配置信息
        for (int index = 0; index < initializeParamNodeInfos.size(); index++) {
            // 批量遍历源路径列表，进行分配
            InitializeParamNodeInfo info = initializeParamNodeInfos.get(index);
            int nextIndex = index * PATH_NUMBER;
            for (int count = 0; count < PATH_NUMBER; nextIndex++, count++) {
                String theSourcePath = theSourcePathList.get(nextIndex);
                int type = StorageVolumeConfig.getStorageTypeFromFileSystemPath(theSourcePath);
                if (type != Integer.MIN_VALUE) {
                    info.putValueByType(type, theSourcePath);
                }
            }
            nodeNameToNodeParmInfos.put(info.getPodName(), info);
        }
    }

    private List<String> preDealPathList(List<String> list) {
        List<String> temp = new ArrayList<>();
        for (int index = 0; index < list.size(); index++) {
            String path = list.get(index);
            if (path != null) {
                path = path.trim();
                if (path.endsWith("/")) {
                    path = path.substring(0, path.length() - 1);
                }
            }
            temp.add(path);
        }
        temp.sort(null);
        return temp;
    }
}