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
package openbackup.access.framework.resource.validator;

import openbackup.access.framework.resource.validator.enums.SchemaKeywordsEnum;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.ErrorCodeConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.JsonSchemaValidatorUtil;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.io.IOException;
import java.io.InputStream;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.concurrent.ConcurrentHashMap;

/**
 * JsonSchema验证器实现类
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-14
 */
@Component
@Slf4j
public class JsonSchemaValidatorImpl implements JsonSchemaValidator {
    private static final String SCHEMA_FILE_PATH_PREFIX = "jsonschema";

    private static final String SCHEMA_FILE_PATH_SUFFIX = "_define.json";

    private static final String TYPE_FIELD = "type";

    private static final String PROPERTIES_FIELD = "properties";

    // 缓存加载的schema文件，key为resourceSubType，value为解析过后的JsonSchema对象
    private static final Map<String, JsonNode> SCHEMAS = new ConcurrentHashMap<>();

    /**
     * 校验
     *
     * @param bean 被校验的对象
     * @param schemaNames JsonSchema文件名字列表，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     * @param <T> 被校验对象泛型
     */
    @Override
    public <T> void doValidate(T bean, String[] schemaNames) {
        if (bean == null) {
            throw new IllegalArgumentException("Checked data should not be null.");
        }
        for (String schemaName : schemaNames) {
            // 校验并获取结果，如果没有定义jsonschema文件，则跳过检查
            if (Objects.nonNull(getSchemaContent(schemaName))) {
                log.info("Validate json schema, schema name: {}", schemaName);
                String fileName = schemaName + SCHEMA_FILE_PATH_SUFFIX;
                try (InputStream inputStream = getSchemaContent(schemaName)) {
                    JsonSchemaValidatorUtil.validate(bean, inputStream, fileName);
                } catch (IOException exception) {
                    log.error("Has no match schema file: {}", fileName);
                }
                return;
            }
        }
    }

    /**
     * 获取加密字段
     *
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     * @return 加密字段
     */
    @Override
    public List<String> getSecretFields(String schemaName) {
        ArrayList<String> schemaSecretFields = new ArrayList<>();
        try (InputStream schemaContent = getSchemaContent(schemaName)) {
            if (schemaContent == null) {
                return Collections.emptyList();
            }

            JsonNode schemaNode = generateJsonNode(schemaContent, schemaName);

            getFieldsByKeyword(schemaNode, SchemaKeywordsEnum.SECRET.getValue(), schemaSecretFields, StringUtils.EMPTY);
            log.debug("Json schema, secret fields size: {}", schemaSecretFields.size());
        } catch (IOException e) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "inputStream occurs error.");
        }

        return schemaSecretFields;
    }

    /**
     * 获取可编辑字段
     *
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     * @return 可编辑字段
     */
    @Override
    public List<String> getEditableFields(String schemaName) {
        List<String> schemaEditableFields = new ArrayList<>();
        InputStream schemaContent = getSchemaContent(schemaName);
        if (schemaContent == null) {
            return Collections.emptyList();
        }

        JsonNode schemaNode = generateJsonNode(schemaContent, schemaName);

        getFieldsByKeyword(schemaNode, SchemaKeywordsEnum.EDITABLE.getValue(), schemaEditableFields, StringUtils.EMPTY);
        log.debug("Json schema, editable fields size: {}", schemaEditableFields.size());
        return schemaEditableFields;
    }

    private InputStream getSchemaContent(String schemaName) {
        // 环境的子类型+下划线+define.json
        String schemaFileName = schemaName + SCHEMA_FILE_PATH_SUFFIX;
        Path path = Paths.get(SCHEMA_FILE_PATH_PREFIX, schemaFileName);

        return getClass().getClassLoader().getResourceAsStream(path.toString());
    }

    private JsonNode generateJsonNode(InputStream schemaContent, String schemaName) {
        if (SCHEMAS.containsKey(schemaName)) {
            return SCHEMAS.get(schemaName);
        }

        JsonNode schemaNode;
        try {
            schemaNode = JSONObject.RAW_OBJ_MAPPER.readTree(schemaContent);
            SCHEMAS.put(schemaName, schemaNode);
            return schemaNode;
        } catch (IOException e) {
            throw new LegoCheckedException(ErrorCodeConstant.SYSTEM_ERROR, e);
        }
    }

    /**
     * JsonSchema对象有嵌套，因此递归遍历JsonSchema，查找secret或者editable字段
     *
     * @param schemaNode JsonSchema对象
     * @param keyword 要查找的字段（在此为secret或者editable）
     * @param queryFields 存放查找到的元素的列表，如果是嵌套对象中存在editable或secret字段，
     *                    会在字段前加上所属关键词前缀，如a.b
     * @param prefix 遍历schema对象时，每一层级的前缀
     */
    private void getFieldsByKeyword(JsonNode schemaNode, String keyword, List<String> queryFields, String prefix) {
        if (schemaNode == null) {
            return;
        }
        // 当前schemaNode不再可能包含有元素名为keyword的内容，则直接返回
        if (!schemaNode.has(keyword) && !schemaNode.has(PROPERTIES_FIELD)) {
            return;
        }

        // 处理当前schemaNode中元素名为keyword的内容，将其所有字段加入到queryFields中
        if (schemaNode.has(keyword)) {
            JsonNode keywordNode = schemaNode.get(keyword);
            // 遍历字段，将字段加上前缀后加入到列表中
            Iterator<JsonNode> elements = keywordNode.elements();
            while (elements.hasNext()) {
                String element = elements.next().textValue();

                if (!VerifyUtil.isEmpty(prefix)) {
                    element = prefix + element;
                }
                queryFields.add(element);
            }
        }

        // 查找当前schemaNode中可能存在元素名为keyword的内容
        JsonNode propertiesNode = schemaNode.get(PROPERTIES_FIELD);
        Iterator<Map.Entry<String, JsonNode>> properties = propertiesNode.fields();

        while (properties.hasNext()) {
            Map.Entry<String, JsonNode> property = properties.next();

            // keyword只能存在与类型为object的元素当中，所以只需处理类型为object的元素
            if (property.getValue().has(TYPE_FIELD)
                    && "object".equals(property.getValue().get(TYPE_FIELD).textValue())) {
                JsonNode curSchemaNode = property.getValue();
                // 将类型为object的元素名作为前缀
                String curPrefix = property.getKey() + "." + prefix;
                getFieldsByKeyword(curSchemaNode, keyword, queryFields, curPrefix);
            }
        }
    }
}
