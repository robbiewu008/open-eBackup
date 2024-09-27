/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.bean;

import lombok.Getter;
import lombok.Setter;

/**
 * OptAuth
 *
 * @author c00106403
 * @version [Lego V100R002C10, 2011-1-7]
 * @since 2020-10-30
 */
@Setter
@Getter
public class OptAuthBo {
    // <变量的意义、目的、功能和可能被用到的地方>
    private static final long serialVersionUID = 4173890973126233693L;

    private long optId = 0L;

    private String optName = "";

    private String url = "";

    // 标识该操作是否是主
    private boolean isMaster = false;

    // 菜单状态（全选，半选，不选）
    private long listOperationStatus = 0L;

    private String serialNumber = "";
}
