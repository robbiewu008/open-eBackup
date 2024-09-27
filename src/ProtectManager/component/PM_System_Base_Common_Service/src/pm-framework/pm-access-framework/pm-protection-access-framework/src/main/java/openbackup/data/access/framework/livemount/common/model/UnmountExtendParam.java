/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.livemount.common.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 挂载销毁扩展参数
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-11-22
 */
@Setter
@Getter
public class UnmountExtendParam {
    private String userId;

    private String exerciseId;

    private String exerciseJobId;

    /**
     * 获取扩展参数实例
     *
     * @return param
     */
    public static UnmountExtendParam getInstance() {
        return new UnmountExtendParam();
    }
}
