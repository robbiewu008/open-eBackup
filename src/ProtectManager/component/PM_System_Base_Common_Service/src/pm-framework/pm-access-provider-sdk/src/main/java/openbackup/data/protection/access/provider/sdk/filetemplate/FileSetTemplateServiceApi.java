/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.filetemplate;

import java.util.List;

/**
 * 文件集模板对外接口
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2022-08-3
 */
public interface FileSetTemplateServiceApi {
    /**
     * 是否存在
     *
     * @param fileSetTemplateId 文件集模块id
     * @return boolean 是否存在
     */
    boolean existFileSetTemplate(String fileSetTemplateId);

    /**
     * 文件集模块id列表
     *
     * @return 文件集模块id列表
     */
    List<String> getFileSetTemplateIdList();
}
