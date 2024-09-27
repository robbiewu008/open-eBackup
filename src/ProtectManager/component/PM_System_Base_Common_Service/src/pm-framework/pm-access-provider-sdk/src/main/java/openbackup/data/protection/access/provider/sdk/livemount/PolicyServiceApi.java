/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.livemount;

import java.util.List;

/**
 * 及时挂载policy对外接口
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2022-07-17
 */
public interface PolicyServiceApi {
    /**
     * query a live mount policy by id
     *
     * @param id id
     * @return boolean 是否存在
     */
    boolean existPolicy(String id);

    /**
     * 策略id列表
     *
     * @return 策略id列表
     */
    List<String> getPolicyIdList();
}
