/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.errors;

/**
 * 资源锁相关错误码定义
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/27
 **/
public abstract class ResourceLockErrorCode {
    /**
     * 原因：资源（{0}）已被锁定或者网络异常，锁定资源失败。
     * 建议：步骤1 请查看错误码原因中是否显示资源ID
     *   如果是=>[步骤2]
     *   如果否=>[步骤5]
     * 步骤2 进入“保护”界面，选择与任务对应的资源类型，找到该资源，并确保该资源没有其他任务正在执行。
     * 步骤3 单击“数据利用 > 副本数据”查询与任务相对应资源类型的副本，找到该副本，并确保该副本没有其他任务正在执行。
     * 步骤4 重新执行任务，查看问题是否解决。
     *  如果是=>处理结束
     *  如果否=>[步骤5]
     * 步骤5 请联系技术支持工程师协助解决。
     */
    public static final long RESOURCE_ALREADY_LOCKED = 1677931286L;

    /**
     * 原因：资源（{0}）有其他任务正在执行或者网络异常，锁定资源失败。
     * 建议：步骤1 稍后重新执行任务，查看问题是否解决。
     *  如果是=>处理结束
     *  如果否=>[步骤2]
     * 步骤2 请联系技术支持工程师协助解决。
     */
    public static final long OCEAN_CYBER_RESOURCE_ALREADY_LOCKED = 1677931340L;
}
