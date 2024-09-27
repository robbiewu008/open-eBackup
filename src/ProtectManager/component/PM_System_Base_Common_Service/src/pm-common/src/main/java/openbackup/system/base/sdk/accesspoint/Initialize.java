/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.accesspoint;

import openbackup.system.base.sdk.accesspoint.model.InitializeParam;
import openbackup.system.base.sdk.accesspoint.model.InitializeResult;
import openbackup.system.base.sdk.accesspoint.model.StandardBackupVolInitInfo;

import java.util.List;

/**
 * 初始化
 *
 * @author w00493811
 * @since 2020-12-26
 */
public interface Initialize {
    /**
     * 初始化备份存储库
     *
     * @param initializeParam 初始化参数
     * @return 初始化结果
     */
    InitializeResult initializeBackStorage(InitializeParam initializeParam);

    /**
     * 获取当前节点挂载的卷信息
     *
     * @return 卷信息列表
     */
    List<StandardBackupVolInitInfo> queryVolumeInfo();
}