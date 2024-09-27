/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-09
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
