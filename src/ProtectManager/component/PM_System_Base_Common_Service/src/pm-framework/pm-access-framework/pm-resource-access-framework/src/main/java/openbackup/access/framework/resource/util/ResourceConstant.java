/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.util;

/**
 * 受保护资源常量
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-04
 */
public class ResourceConstant {
    /**
     * 手动扫描资源的 schedule
     */
    public static final String MANUAL_SCAN_RESOURCE = "manual_scan_resource";

    /**
     * job schedule前缀
     */
    public static final String JOB_SCHEDULE_PREFIX = "job_schedule_";

    /**
     * job schedule前缀
     */
    public static final String JOB_TYPE_PREFIX = "job_type_";

    /**
     * 本地存储文件系统
     */
    public static final String LOCAL_STORAGE_FILE_SYSTEM = "LocalStorage";

    /**
     * 消息中的资源id key
     */
    public static final String RES_ID = "resId";

    /**
     * 消息中的job_id
     */
    public static final String JOB_ID = "job_id";

    /**
     * 消息中的job_id
     */
    public static final String SUBTYPE = "subtype";

    /**
     * redis资源锁的key前缀, 用于资源扫描和资源删除
     */
    public static final String RESOURCE_LOCK_KEY = "/resource/lock/";

    /**
     * 资源扫描锁的等待时间
     */
    public static final int RESOURCE_SCAN_LOCK_WAIT_TIME = 5;
}
