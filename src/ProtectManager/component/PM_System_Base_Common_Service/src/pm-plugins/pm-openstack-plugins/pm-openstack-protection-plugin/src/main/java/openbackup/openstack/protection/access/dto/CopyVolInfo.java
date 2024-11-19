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
package openbackup.openstack.protection.access.dto;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;

import lombok.Getter;
import lombok.Setter;
import openbackup.data.access.framework.core.common.model.AbstractVmVolInfo;
import openbackup.data.access.framework.core.common.model.CopySnapShotInfo;
import openbackup.data.access.framework.core.common.model.DiskInfo;
import openbackup.data.access.framework.core.common.model.VmIndexerCopyMetaData;
import openbackup.data.access.framework.core.common.model.VmSnapMetadata;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.logging.log4j.util.Strings;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * CopyVolInfo
 *
 */
@Getter
@Setter
public class CopyVolInfo extends AbstractVmVolInfo {
    private static final String EMPTY_LIST = JSONObject.stringify(new VmSnapMetadata(new ArrayList<>()));

    private String uuid;

    private String name;

    private List<Map<String, String>> attachments;

    /**
     * 索引磁盘信息转换方法
     *
     * @param properties 配置文件
     * @return 索引磁盘信息
     */
    public static String convert2IndexDiskInfos(JSONObject properties) {
        if (VerifyUtil.isEmpty(properties)) {
            return EMPTY_LIST;
        }
        List<CopySnapShotInfo> copySnapShotInfos = obtainSnapshotInfos(properties);
        String diskJson = properties.getString(VmIndexerCopyMetaData.VOL_LIST);
        if (VerifyUtil.isEmpty(diskJson)) {
            return EMPTY_LIST;
        }
        List<CopyVolInfo> fcVolInfos = JSON.parseObject(diskJson, new TypeReference<List<CopyVolInfo>>() {});

        // 由于ubc对name有顺序要求, diskInfos的name默认取attachments里的device
        List<DiskInfo> diskInfos = fcVolInfos.stream()
            .map(fcVolInfo -> DiskInfo.builder()
                .guid(fcVolInfo.getUuid())
                .name((!VerifyUtil.isEmpty(fcVolInfo.getAttachments())
                        && !Strings.isEmpty(fcVolInfo.getAttachments().get(0).get("device")))
                        ? fcVolInfo.getAttachments().get(0).get("device") : fcVolInfo.getName())
                .fileSystemName(copySnapShotInfos.get(0).getParentName())
                .snapshotId(copySnapShotInfos.get(0).getId())
                .snapshotName(copySnapShotInfos.get(0).splitForSnapshotName())
                .build())
            .collect(Collectors.toList());
        return JSONObject.stringify(new VmSnapMetadata(diskInfos));
    }
}
