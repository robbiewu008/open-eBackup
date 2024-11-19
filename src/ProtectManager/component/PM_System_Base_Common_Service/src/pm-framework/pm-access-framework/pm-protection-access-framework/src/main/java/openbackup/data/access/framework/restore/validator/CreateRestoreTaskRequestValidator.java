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
package openbackup.data.access.framework.restore.validator;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.system.base.controller.validator.BaseParamValidator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import org.springframework.stereotype.Component;
import org.springframework.validation.Errors;

/**
 * 创建恢复任务请求的自定义业务校验器
 *
 **/
@Slf4j
@Component
public class CreateRestoreTaskRequestValidator extends BaseParamValidator<CreateRestoreTaskRequest> {
    private final CopyRestApi copyRestApi;

    public CreateRestoreTaskRequestValidator(CopyRestApi copyRestApi) {
        this.copyRestApi = copyRestApi;
    }

    @Override
    public boolean supports(Class<?> clazz) {
        return CreateRestoreTaskRequest.class.isAssignableFrom(clazz);
    }

    @Override
    protected void customValidate(CreateRestoreTaskRequest request, Errors errors) {
        final Copy copy = copyRestApi.queryCopyByID(request.getCopyId());
        request.setCopy(copy);
    }
}
