/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.framework.core.common.constants;

import com.google.common.collect.ImmutableList;
import com.google.common.collect.ImmutableSet;

import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.List;
import java.util.Set;

/**
 * Copy index constant
 *
 */
public final class CopyIndexConstants {
    /**
     * response status flag of index
     */
    public static final String STATUS = "status";

    /**
     * response flag of sucess
     */
    public static final String SUCCESS = "success";

    /**
     * response flag of abort
     */
    public static final String ABORT = "abort";

    /**
     * index json file path
     */
    public static final String COPY_INDEX_FILE_PATH = "/tmp/share/rfi_files/";

    /**
     * underscore separator
     */
    public static final String UNDERSCORE_SEPARATOR = "_";

    /**
     * file separator
     */
    public static final String FILE_SEPARATOR = "/";

    /**
     * index file prefix
     */
    public static final String INDEX_FILE_PREFIX = "RFI";

    /**
     * index file prefix
     */
    public static final String INDEX_FILE_SUFFIX = ".json";

    /**
     * index file prefix
     */
    public static final String INDEX_ZIP_SUFFIX = ".zip";

    /**
     * response status flag of index
     */
    public static final String PATH = "path";

    /**
     * fine grained restore
     */
    public static final String FINE_GRAINED_RESTORE = "fine_grained_restore";

    /**
     * supported index resource
     */
    public static final List<String> SUPPORT_INDEX_RESOURCE = ImmutableList.of(ResourceSubTypeEnum.VMWARE.getType(),
        ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType(), ResourceSubTypeEnum.HCS_CLOUD_HOST.getType(),
        ResourceSubTypeEnum.FUSION_COMPUTE.getType(), ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType(),
        ResourceSubTypeEnum.APS_INSTANCE.getType(), ResourceSubTypeEnum.CNWARE_VM.getType(),
        ResourceSubTypeEnum.NUTANIX_VM.getType(), ResourceSubTypeEnum.HYPER_V_VM.getType(),
        ResourceSubTypeEnum.NDMP_BACKUPSET.getType(), ResourceSubTypeEnum.FUSION_ONE_COMPUTE.getType());

    /**
     * supported index generated by
     */
    public static final List<String> NOT_SUPPORT_INDEX_GENERATED_BY = ImmutableList.of(
        CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(), CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());

    /**
     * 需要创建索引，并且需要检查SLA配置的资源集合
     */
    public static final Set<String> NEED_TO_CHECK_SLA_CONFIG_INDEX_RESOURCE =
        ImmutableSet.of(ResourceSubTypeEnum.VMWARE.getType(), ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType(),
            ResourceSubTypeEnum.HCS_CLOUD_HOST.getType(), ResourceSubTypeEnum.FUSION_COMPUTE.getType(),
            ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType(), ResourceSubTypeEnum.APS_INSTANCE.getType(),
            ResourceSubTypeEnum.CNWARE_VM.getType(), ResourceSubTypeEnum.NUTANIX_VM.getType(),
            ResourceSubTypeEnum.HYPER_V_VM.getType(), ResourceSubTypeEnum.NDMP_BACKUPSET.getType(),
            ResourceSubTypeEnum.FUSION_ONE_COMPUTE.getType());

    /**
     * 需要创建索引，并且存储上下文
     */
    public static final Set<String> NEED_TO_SAVE_CONTEXT_INDEX_RESOURCE =
        ImmutableSet.of(ResourceSubTypeEnum.VMWARE.getType(), ResourceSubTypeEnum.CLOUD_BACKUP_FILE_SYSTEM.getType(),
            ResourceSubTypeEnum.HCS_CLOUD_HOST.getType(), ResourceSubTypeEnum.FUSION_COMPUTE.getType(),
            ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType(), ResourceSubTypeEnum.APS_INSTANCE.getType(),
            ResourceSubTypeEnum.CNWARE_VM.getType(), ResourceSubTypeEnum.NUTANIX_VM.getType(),
            ResourceSubTypeEnum.HYPER_V_VM.getType(), ResourceSubTypeEnum.NDMP_BACKUPSET.getType(),
            ResourceSubTypeEnum.FUSION_ONE_COMPUTE.getType());

    /**
     * manual index
     */
    public static final String GEN_INDEX_MANUAL = "manual";

    /**
     * index operation type
     */
    public static final String INDEX_OPERATE_TYPE = "gen_index";

    /**
     * index colume name
     */
    public static final String INDEX_COLUME = "indexed";

    /**
     * flr switch on
     */
    public static final String FLR_SWITCH_ON = "true";

    /**
     * copy type
     */
    public static final String BACK_UP = "backup";

    /**
     * 自动索引
     */
    public static final String AUTO_INDEX = "auto_index";

    /**
     * 文件细粒度恢复
     */
    public static final String FLR_RESTORE_TYPE = "FLR";

    /**
     * 文件细粒度下载
     */
    public static final String FLR_DOWNLOAD = "download";

    /**
     * 设备esn
     */
    public static final String DEVICE_ESN = "DEVICE_ESN";
}
