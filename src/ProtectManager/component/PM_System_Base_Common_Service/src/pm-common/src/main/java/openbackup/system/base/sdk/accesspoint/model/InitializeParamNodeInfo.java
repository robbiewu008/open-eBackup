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

import org.apache.commons.lang3.StringUtils;

import java.util.Comparator;
import java.util.Map;

/**
 * 初始化参数路径
 *
 */
@Data
@NoArgsConstructor
@JsonInclude(JsonInclude.Include.NON_NULL)
public class InitializeParamNodeInfo {
    private String podName;

    private Map<String, String> abAddressToNasMap;

    private String nasPathForStandardVolume;

    private String nasPathForMetaDataVolume;

    private String nasPathForSelfBackVolume;

    private String nasPathForCloudIdxVolume;

    private String nasSourcePath;

    /**
     * 构造函数
     *
     * @param podName POD名称
     * @param abAddressToNasMap 爱数地址->NAS地址（服务器）图
     */
    public InitializeParamNodeInfo(String podName, Map<String, String> abAddressToNasMap) {
        setPodName(podName);
        setAbAddressToNasMap(abAddressToNasMap);
    }

    /**
     * 获取排序器
     *
     * @return 排序器
     */
    public static final Comparator<InitializeParamNodeInfo> getComparator() {
        return Comparator.comparing(InitializeParamNodeInfo::getPodName);
    }

    /**
     * 是否已经被全部设定
     *
     * @return 已经被全部设定
     */
    public boolean isAllSet() {
        return StringUtils.isNoneBlank(nasPathForCloudIdxVolume, nasPathForMetaDataVolume, nasPathForSelfBackVolume,
            nasPathForStandardVolume, nasSourcePath);
    }

    /**
     * 塞值
     *
     * @param type 类型
     * @param path 路径
     */
    public void putValueByType(int type, String path) {
        if (type == StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP) {
            setNasPathForStandardVolume(path);
        } else if (type == StorageVolumeConfig.VOLUME_TYPE_METADATA_BACKUP) {
            setNasPathForMetaDataVolume(path);
        } else if (type == StorageVolumeConfig.VOLUME_TYPE_COULD_BACKUP) {
            setNasPathForCloudIdxVolume(path);
        } else if (type == StorageVolumeConfig.VOLUME_TYPE_SELF_BACKUP) {
            setNasPathForSelfBackVolume(path);
        } else if (type == StorageVolumeConfig.VOLUME_TYPE_FILE_SYSTEM) {
            setNasSourcePath(path);
        } else {
            return;
        }
    }

    /**
     * 我方路径是否含有对方路径
     *
     * @param theSourceInitializeParamNodeInfo 对方路径信息
     * @return 对方路径是否含有本方路径
     */
    public boolean isContainsPath(InitializeParamNodeInfo theSourceInitializeParamNodeInfo) {
        return isEquals(theSourceInitializeParamNodeInfo.nasPathForStandardVolume) || isEquals(
            theSourceInitializeParamNodeInfo.nasPathForMetaDataVolume) || isEquals(
            theSourceInitializeParamNodeInfo.nasPathForSelfBackVolume) || isEquals(
            theSourceInitializeParamNodeInfo.nasPathForCloudIdxVolume);
    }

    /**
     * 我方路径是否含有NAS源路径
     *
     * @param sourceNasPath NAS源路径(/dpa_fs ...)
     * @return 我方路径是否含有NAS源路径
     */
    public boolean isContainsPath(String sourceNasPath) {
        if (StringUtils.isEmpty(sourceNasPath)) {
            return false;
        } else {
            return contains(nasPathForStandardVolume, sourceNasPath) || contains(nasPathForMetaDataVolume,
                sourceNasPath) || contains(nasPathForSelfBackVolume, sourceNasPath) || contains(
                nasPathForCloudIdxVolume, sourceNasPath);
        }
    }

    private boolean contains(String source, String target) {
        if (StringUtils.isEmpty(source)) {
            return false;
        } else {
            return (source + "/").contains(target);
        }
    }

    /**
     * 我方路径是否含有NAS源路径
     *
     * @param sourceNasPath NAS源路径(/dpa_fs ...)
     * @return 我方路径是否含有NAS源路径
     */
    public boolean isEquals(String sourceNasPath) {
        if (StringUtils.isEmpty(sourceNasPath)) {
            return false;
        } else {
            return equals(nasPathForStandardVolume, sourceNasPath) || equals(nasPathForMetaDataVolume, sourceNasPath)
                || equals(nasPathForSelfBackVolume, sourceNasPath) || equals(nasPathForCloudIdxVolume, sourceNasPath);
        }
    }

    private boolean equals(String source, String target) {
        if (StringUtils.isEmpty(source)) {
            return false;
        } else {
            return source.contains(target);
        }
    }
}
