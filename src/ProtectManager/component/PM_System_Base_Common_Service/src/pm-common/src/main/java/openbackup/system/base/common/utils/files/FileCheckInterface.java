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
package openbackup.system.base.common.utils.files;

import openbackup.system.base.bean.FileCheckRule;

import org.springframework.web.multipart.MultipartFile;

/**
 * 文件校验接口
 *
 * @author w00607005
 * @since 2023-09-21
 */
public interface FileCheckInterface {
    /**
     * 检验文件
     *
     * @param multipartFile 文件
     * @param validateRule 校验规则
     *
     * @return 校验结果
     */
    boolean check(MultipartFile multipartFile, FileCheckRule validateRule);
}
