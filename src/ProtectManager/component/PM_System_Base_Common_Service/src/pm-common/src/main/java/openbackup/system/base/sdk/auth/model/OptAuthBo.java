/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

/**
 * OptAuth
 *
 * @author c00106403
 * @version [Lego V100R002C10, 2011-1-7]
 * @since 2018-10-30
 */
@Setter
@Getter
public class OptAuthBo {
    private long ioptID = 0L;

    private String soptName = "";

    private String surl = "";

    // 标识该操作是否是主
    private boolean imaster = false;

    // 菜单状态（全选，半选，不选）
    private long loperationStatus = 0L;

    private String serialNumber = "";
}
