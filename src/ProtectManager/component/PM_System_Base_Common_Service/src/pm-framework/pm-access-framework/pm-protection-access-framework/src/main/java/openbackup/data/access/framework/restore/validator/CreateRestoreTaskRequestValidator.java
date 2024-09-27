/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.restore.validator;

import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.system.base.controller.validator.BaseParamValidator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;
import org.springframework.validation.Errors;

/**
 * 创建恢复任务请求的自定义业务校验器
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/3/5
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
