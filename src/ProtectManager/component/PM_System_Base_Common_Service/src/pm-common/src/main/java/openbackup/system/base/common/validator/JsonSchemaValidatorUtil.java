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
package openbackup.system.base.common.validator;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.everit.json.schema.Schema;
import org.everit.json.schema.ValidationException;
import org.everit.json.schema.loader.SchemaClient;
import org.everit.json.schema.loader.SchemaLoader;
import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONTokener;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * JsonSchemaUtil
 *
 */
@Slf4j
public class JsonSchemaValidatorUtil {
    /**
     * 执行JsonSchema校验
     *
     * @param bean 需校验对象
     * @param filePaths JsonSchema全路径文件名列表
     * @param <T> 需校验对象类型
     */
    public static <T> void validate(T bean, String[] filePaths) {
        for (String filePath : filePaths) {
            validate(bean, filePath);
        }
    }

    /**
     * 执行JsonSchema校验
     *
     * @param bean 需校验对象
     * @param filePath JsonSchema全路径文件名
     * @param <T> 需校验对象类型
     */
    public static <T> void validate(T bean, String filePath) {
        try (InputStream inputStream = JsonSchemaValidatorUtil.class.getClassLoader().getResourceAsStream(filePath)) {
            validate(bean, inputStream, filePath);
        } catch (IOException exception) {
            log.error("No json schema file found: {}", filePath);
        }
    }

    /**
     * 执行校验
     *
     * @param <T> 需校验对象类型
     * @param bean 需校验的对象
     * @param fileName 定义的JsonSchema文件的文件名
     * @param inputStream schema文件流
     */
    public static <T> void validate(T bean, InputStream inputStream, String fileName) {
        validate(bean, fileName, inputStream, false);
    }

    /**
     * 执行校验逻辑
     *
     * @param bean 需校验对象
     * @param fileName 定义的JsonSchema文件的文件名
     * @param isStrict jsonschema文件不存在时，是否抛出异常
     * @param inputStream schema文件流
     * @param <T> 需校验对象的类型
     */
    public static <T> void validate(T bean, String fileName, InputStream inputStream, boolean isStrict) {
        // JsonSchema文件
        Optional<Schema> jsonSchema = generateSchema(fileName, inputStream, isStrict);
        jsonSchema.ifPresent(schema -> {
            log.info("Find match json schema file: {}", fileName);
            // 待校验的json
            try {
                JSONObject json = generateJsonObject(bean);
                schema.validate(json);
            } catch (ValidationException e) {
                // 处理异常信息
                processErrorMessage(e);
            }
        });
    }

    private static Optional<Schema> generateSchema(String fileName, InputStream inputStream, boolean isStrict) {
        log.info("Generate schema. schema file name:{}", fileName);
        if (inputStream == null) {
            log.error("Has no match schema file: {}", fileName);
            if (isStrict) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Has no match schema file");
            }
        } else {
            JSONTokener schemaData = new JSONTokener(inputStream);
            JSONObject jsonSchema = new JSONObject(schemaData);
            SchemaLoader loader = SchemaLoader.builder()
                .schemaClient(SchemaClient.classPathAwareClient())
                .schemaJson(jsonSchema)
                .build();
            Schema schema = loader.load().build();
            return Optional.ofNullable(schema);
        }
        return Optional.empty();
    }

    private static <T> JSONObject generateJsonObject(T bean) {
        try {
            if (bean == null) {
                return new JSONObject();
            }
            if (bean instanceof String || bean instanceof JsonNode) {
                return new JSONObject(bean.toString());
            }
            return new JSONObject(
                openbackup.system.base.common.utils.JSONObject.RAW_OBJ_MAPPER.valueToTree(bean)
                    .toString());
        } catch (JSONException ex) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, ExceptionUtil.getErrorMessage(ex));
        }
    }

    // 处理异常信息
    private static void processErrorMessage(ValidationException exception) {
        List<String> errorMessages = processSubMessages(exception);
        String errors = String.join(";", errorMessages);
        if (errorMessages.size() != IsmNumberConstant.ZERO) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, errors);
        }
    }

    // 处理子错误信息
    private static List<String> processSubMessages(ValidationException exception) {
        List<String> list = new ArrayList<>();
        List<ValidationException> causingExceptions = exception.getCausingExceptions();
        if (causingExceptions != null && causingExceptions.size() > LegoNumberConstant.ONE) {
            for (ValidationException causingException : causingExceptions) {
                List<String> subErrorMessages = processSubMessages(causingException);
                list.addAll(subErrorMessages);
            }
        } else {
            list.add(processValidationException(exception));
        }
        return list;
    }

    // 处理单个ValidationException异常信息
    private static String processValidationException(ValidationException exception) {
        StringBuilder errorMsgBuilder = new StringBuilder();
        errorMsgBuilder.append(" ErrorKey: ")
            .append(exception.getPointerToViolation() == null ? StringUtils.SPACE : exception.getPointerToViolation())
            .append(", ErrorMessage: ")
            .append(exception.getErrorMessage());
        return errorMsgBuilder.toString();
    }
}
