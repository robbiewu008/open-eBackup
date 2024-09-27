/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.service;

/**
 * 更新ip路由服务
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/2/27
 */
public interface IpRuleService {
    /**
     * 在底座上给网卡添加到指定目的ip的路由
     *
     * @param destinationIp 目的地ip
     * @param taskType 任务类型
     */
    void addIpRule(String destinationIp, String taskType);

    /**
     * 在底座上删除到指定目的ip的路由
     *
     * @param destinationIp 目的地ip
     * @param taskType 任务类型
     */
    void deleteIpRule(String destinationIp, String taskType);

    /**
     * 只在本控上添加路由
     *
     * @param destinationIp 目的地ip
     * @param taskType 任务类型
     */
    void addIpRuleLocal(String destinationIp, String taskType);

    /**
     * 只在本控上删除路由
     *
     * @param destinationIp 目的地ip
     * @param taskType 任务类型
     */
    void deleteIpRuleLocal(String destinationIp, String taskType);
}
