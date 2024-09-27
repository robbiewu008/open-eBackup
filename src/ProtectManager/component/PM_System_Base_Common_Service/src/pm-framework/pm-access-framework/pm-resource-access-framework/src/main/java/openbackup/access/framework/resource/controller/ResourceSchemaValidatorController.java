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
package openbackup.access.framework.resource.controller;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.system.base.security.exterattack.ExterAttack;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.List;

/**
 * JsonSchema校验器REST API控制器，提供JsonSchema校验器相关REST接口
 *
 * @author w00616953
 * @since 2021-10-20
 */
@RestController
@RequestMapping("/v2/internal/jsonschemas")
@Slf4j
public class ResourceSchemaValidatorController {
    @Autowired
    private JsonSchemaValidator jsonSchemaValidator;

    /**
     * 对数据进行校验
     *
     * @param checkedData 需要校验的数据
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     */
    @ExterAttack
    @GetMapping(value = "/{schemaName}/validation/result")
    public void doValidate(@RequestBody JsonNode checkedData, @PathVariable("schemaName") String schemaName) {
        jsonSchemaValidator.doValidate(checkedData, schemaName);
    }

    /**
     * 获取可编辑的字段
     *
     * @param schemaName JsonSchema文件名字
     * @return 返回存有可编辑字段的列表
     */
    @ExterAttack
    @GetMapping("/{schemaName}/fields/editable")
    public List<String> getEditableFields(@PathVariable("schemaName") String schemaName) {
        return jsonSchemaValidator.getEditableFields(schemaName);
    }

    /**
     * 获取敏感字段
     *
     * @param schemaName JsonSchema文件名字
     * @return 返回存有敏感字段的列表
     */
    @ExterAttack
    @GetMapping("/{schemaName}/fields/secret")
    public List<String> getSecretFields(@PathVariable("schemaName") String schemaName) {
        return jsonSchemaValidator.getSecretFields(schemaName);
    }
}
