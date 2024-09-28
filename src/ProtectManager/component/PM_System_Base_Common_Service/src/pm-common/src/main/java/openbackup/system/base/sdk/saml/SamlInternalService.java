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
package openbackup.system.base.sdk.saml;

import org.springframework.web.multipart.MultipartFile;

import javax.servlet.http.HttpServletRequest;

/**
 * Saml服务
 *
 */
public interface SamlInternalService {
    /**
     * 获取saml metadata
     *
     * @param httpServletRequest 请求头
     * @return metadata数据
     */
    String getMarshallMetadata(HttpServletRequest httpServletRequest);

    /**
     * 校验metadata文件是否合法
     *
     * @param multipartFile 上传的文件
     */
    void validateIPDMetadata(MultipartFile multipartFile);
}
