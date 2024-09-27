package openbackup.data.access.framework.core.common.model;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import lombok.extern.slf4j.Slf4j;

import org.springframework.util.CollectionUtils;

import java.util.List;

/**
 * Vm卷信息抽象类
 *
 * @author y30037959
 * @since 2023-06-13
 */
@Slf4j
public abstract class AbstractVmVolInfo {
    /**
     * 获取快照信息
     *
     * @param properties 配置信息
     * @return 快照信息
     */
    protected static List<CopySnapShotInfo> obtainSnapshotInfos(JSONObject properties) {
        log.info("start parse disk info");
        List<CopySnapShotInfo> copySnapShotInfos = JSONArray.toCollection(
            properties.getJSONArray(VmIndexerCopyMetaData.SNAPSHOTS), CopySnapShotInfo.class);
        if (CollectionUtils.isEmpty(copySnapShotInfos)) {
            log.error("snapshot info empty create scan request fail");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "snapshot info empty create scan request fail");
        }
        return copySnapShotInfos;
    }
}
