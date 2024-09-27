/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.core.common.constants;

/**
 * 副本处理过程中需要的常量
 *
 * @version [BCManager 8.0.0]
 * @author p00511147
 * @since 2020-10-26
 **/
public class CopyConstants {
    /**
     * 副本id
     */
    public static final String BACKUP_ID = "backup_id";

    /**
     * 副本是否过期
     */
    public static final String COPY_EXPIRE = "COPY_EXPIRE";

    /**
     * 成功状态码
     */
    public static final String SUCCESS_CODE = "0";

    /**
     * chainId
     */
    public static final String CHAIN_ID = "chain_id";

    /**
     * ebackup copy
     */
    public static final String E_BACKUP_COPY = "eBackup_copy";

    /**
     * resource
     */
    public static final String RESOURCE = "resource";

    /**
     * "sla"
     */
    public static final String SLA = "sla";

    /**
     * "policy"备份策略
     */
    public static final String POLICY = "policy";

    /**
     * 副本名称在redis 里面的key
     */
    public static final String COPY_NAME = "copy_name";
}
