/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.opengauss.resources.access.constants;

/**
 * OpenGauss相关错误码
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-21
 */
public class OpenGaussErrorCode {
    /**
     * 错误场景：执行注册/修改应用集群操作时，由于选择的集群类型与应用集群类型不匹配，操作失败。
     * 原因：选择的集群类型与应用集群类型不匹配。
     * 建议：请选择与集群类型相匹配的应用后重试。
     */
    public static final long CLUSTER_CLUSTER_TYPE_INCONSISTENT = 1577209995L;

    /**
     * 场景：注册/修改应用集群操作，检测注册信息成功。
     */
    public static final long SUCCESS = 0L;
}
