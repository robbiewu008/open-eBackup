/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.core.common.model;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.Getter;
import lombok.Setter;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

/**
 * 统一框架快照信息
 *
 * @author y30037959
 * @since 2023-06-07
 */
@Slf4j
@Getter
@Setter
public class CopySnapShotInfo {
    private String id;

    private String name;

    private String parentName;

    /**
     * 根据snapshotId生成snapshot名称
     * snapshotId格式（filesystemId + @ + snapshotName）
     *
     * @return 快照名称
     */
    public String splitForSnapshotName() {
        log.info("start to split snapshotId:{}", id);
        if (StringUtils.isBlank(id)) {
            return StringUtils.EMPTY;
        }
        String[] names = id.split("@");
        if (names.length != 2) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "split for snapshot name error");
        }
        return names[1];
    }
}
