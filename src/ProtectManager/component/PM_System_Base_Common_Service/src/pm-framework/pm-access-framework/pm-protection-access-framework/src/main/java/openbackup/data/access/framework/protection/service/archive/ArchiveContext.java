/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.archive;

import openbackup.system.base.sdk.protection.model.PolicyBo;

import java.util.Optional;

/**
 * 归档任务上下文统一抽象接口
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/14
 **/
public interface ArchiveContext {
    /**
     * 归档策略key定义
     */
    String ARCHIVE_POLICY_KEY = "policy";

    /**
     * 归档策略扩展参数key定义
     */
    String ARCHIVE_POLICY_EXT_PARAMS_KEY = "ext_parameters";

    /**
     * sla 名称key定义
     */
    String SLA_NAME_KEY = "name";

    /**
     * 手动归档标志
     */
    String MANUAL_ARCHIVE = "manual_archive";

    /**
     * sla key定义
     */
    String SLA_KEY = "sla";

    /**
     * 备份副本id可以
     */
    String ORIGINAL_COPY_ID_KEY = "copy_id";

    /**
     * 归档副本id key
     */
    String ARCHIVE_COPY_ID_KEY = "id";

    /**
     * 归档任务job id
     */
    String JOB_ID = "job_id";

    /**
     * 归档任务自动重试次数
     */
    String AUTO_RETRY_TIMES = "auto_retry_times";

    /**
     * 从上下文中获取策略信息
     *
     * @return 策略信息对象{@code PolicyBo}
     */
    PolicyBo getPolicy();

    /**
     * 获取归档策略高级参数
     *
     * @return 归档策略高级参数
     */
    String getPolicyExtParams();

    /**
     * 从上下文中获取SLA的json格式字符串
     *
     * @return sla json串
     */
    String getSlaJson();

    /**
     * 从上下文中获取sla名称
     *
     * @return sla名称{@code SlaBo}
     */
    String getSlaName();

    /**
     * 从上下文中获取副本id
     *
     * @return 副本id
     */
    boolean getManualArchiveTag();


    /**
     * 从上下文中获取原副本id
     *
     * @return 备份副本id
     */
    String getOriginalCopyId();

    /**
     * 设置归档副本id到上下文
     *
     * @param archiveCopyId 归档副本id
     * @return 之前上下文中的值
     */
    String setArchiveCopyId(String archiveCopyId);

    /**
     * 从上下文中获取归档副本id
     *
     * @return 归档副本id
     */
    Optional<String> getArchiveCopyId();

    /**
     * 从上下文中获取job id
     *
     * @return 归档job id
     */
    String getJobId();

    /**
     * 从上下文中获归档任务的重试次数
     *
     * @return 归档任务重试次数
     */
    int getRetryTimes();
}
