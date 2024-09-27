package openbackup.openstack.protection.access.dto;

import openbackup.data.access.framework.core.common.model.AbstractVmVolInfo;
import openbackup.data.access.framework.core.common.model.CopySnapShotInfo;
import openbackup.data.access.framework.core.common.model.DiskInfo;
import openbackup.data.access.framework.core.common.model.VmIndexerCopyMetaData;
import openbackup.data.access.framework.core.common.model.VmSnapMetadata;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.TypeReference;

import lombok.Getter;
import lombok.Setter;

import org.apache.logging.log4j.util.Strings;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * CopyVolInfo
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-31
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
