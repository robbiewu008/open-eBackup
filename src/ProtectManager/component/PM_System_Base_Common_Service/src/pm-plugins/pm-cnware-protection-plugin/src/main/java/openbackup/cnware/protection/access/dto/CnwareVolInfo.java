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
package openbackup.cnware.protection.access.dto;

import openbackup.data.access.framework.core.common.model.AbstractVmVolInfo;
import openbackup.data.access.framework.core.common.model.CopySnapShotInfo;
import openbackup.data.access.framework.core.common.model.DiskInfo;
import openbackup.data.access.framework.core.common.model.VmIndexerCopyMetaData;
import openbackup.data.access.framework.core.common.model.VmSnapMetadata;
import openbackup.system.base.common.utils.JSONObject;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 */
@Setter
@Getter
public class CnwareVolInfo extends AbstractVmVolInfo {
    private String bootable;

    private String extendInfo;

    @JsonProperty("ifNewVolume")
    private boolean isNewVolume;

    private String location;

    private String metadata;

    private String moRef;

    private String name;

    private String slotId;

    private String type;

    private String uuid;

    private String vmMoRef;

    private long volSizeInBytes;

    private String volumeType;

    /**
     * 索引磁盘信息转换方法
     *
     * @param properties 配置文件
     * @return 索引磁盘信息
     */
    public static String convert2IndexDiskInfos(JSONObject properties) {
        List<CopySnapShotInfo> copySnapShotInfos = obtainSnapshotInfos(properties);
        String diskJson = properties.getString(VmIndexerCopyMetaData.VOL_LIST);
        List<CnwareVolInfo> cnwareVolInfoList =
            JSON.parseObject(diskJson, new TypeReference<List<CnwareVolInfo>>() {});
        List<DiskInfo> diskInfos =
            cnwareVolInfoList.stream().map(cnwareVolInfo -> DiskInfo.builder()
                .guid(cnwareVolInfo.getUuid())
                .name(cnwareVolInfo.getName())
                .fileSystemName(copySnapShotInfos.get(0).getParentName())
                .snapshotId(copySnapShotInfos.get(0).getId())
                .snapshotName(copySnapShotInfos.get(0).splitForSnapshotName())
                .build())
            .collect(Collectors.toList());
        return JSONObject.stringify(new VmSnapMetadata(diskInfos));
    }
}
