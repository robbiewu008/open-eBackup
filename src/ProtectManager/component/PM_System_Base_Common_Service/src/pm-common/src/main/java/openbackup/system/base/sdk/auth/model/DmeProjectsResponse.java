/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 查询dme用户关联的项目列表返回体
 *
 * @author z30062305
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-29
 */
@Setter
@Getter
public class DmeProjectsResponse {
    /**
     * 查询的用户关联资源集的总数
     */
    private Integer total;

    /**
     * dme资源集列表
     */
    private List<DmeProject> projects;
}
