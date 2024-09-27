/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.livemount.common.constants;

/**
 * LiveMountConstants LiveMount常量
 *
 * @author twx1009756
 * @version [OceanProtectA8000 1.1.0]
 * @since 2022.3.23
 */
public class LiveMountConstants {
    /**
     * nas文件系统的share信息
     */
    public static final String FILE_SYSTEM_SHARE_INFO = "fileSystemShareInfo";

    /**
     * 用户是否手动执行操作挂载操作的KRY值
     */
    public static final String MANUAL_MOUNT = "manualMount";

    /**
     * 用户是否手动执行操作挂载操作的value值
     */
    public static final String TRUE = "true";

    /**
     * 即时挂载的任务id
     */
    public static final String MOUNT_JOB_ID = "mountJobId";

    /**
     * 即时挂载流程中生成的克隆副本ID存放到缓存中的Key
     */
    public static final String CLONE_COPY_ID = "cloneCopyId";

    /**
     * 文件系统子类型
     */
    public static final String FILE_SUB_TYPE = "fileSubType";

    /**
     * worm类型值
     */
    public static final String SUB_TYPE_WORM = "1";

    /**
     * reserve copy
     */
    public static final String RESERVE_COPY = "reserve_copy";

    /**
     * reserve copy
     */
    public static final String FORCE_DELETE = "force_delete";
}

