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

import java.util.List;

/**
 * JsonSchema验证接口定义
 *
 */
public interface JsonSchemaValidator {
    /**
     * 根据json schema校验传入对象的合法性
     *
     * @param <T> 被校验对象的类型
     * @param bean 被校验的对象
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     */
    default <T> void doValidate(T bean, String schemaName) {
        this.doValidate(bean, new String[] {schemaName});
    }

    /**
     * 根据json schema校验传入对象的合法性
     *
     * @param <T> 被校验对象的类型
     * @param bean 被校验的对象
     * @param schemaNames JsonSchema文件名字列表
     */
    <T> void doValidate(T bean, String[] schemaNames);

    /**
     * 返回被校验对象中，类型为secret的字段
     *
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     * @return 被校验对象中，类型为secret的字段列表
     */
    List<String> getSecretFields(String schemaName);

    /**
     * 返回被校验对象中，可被编辑的字段
     *
     * @param schemaName JsonSchema文件名字，按照统一规定给出，如资源相关的文件名为resourceSubType_define.json
     * @return 被校验对象中，可被编辑的字段列表
     */
    List<String> getEditableFields(String schemaName);
}
