/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.model.hcs;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 功能描述 主机扩展信息
 *
 * @author z30027603
 * @since 2022/7/22 9:52
 */
@Getter
@Setter
public class HostExtendInfo {
    private String id;

    private String name;

    private String regionId;

    private String projectId;

    private String az;

    private String status;

    private List<DiskInfo> diskInfo;
}
