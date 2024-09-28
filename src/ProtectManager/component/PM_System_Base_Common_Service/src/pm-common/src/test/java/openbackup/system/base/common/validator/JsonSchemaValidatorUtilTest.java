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

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.validator.JsonSchemaValidatorUtil;

import org.apache.commons.io.FileUtils;
import org.everit.json.schema.Schema;
import org.json.JSONObject;
import org.junit.Assert;
import org.junit.Test;
import org.powermock.reflect.Whitebox;
import org.springframework.util.ResourceUtils;

import java.io.File;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Optional;

/**
 * {@link JsonSchemaValidatorUtil 测试类}
 *
 */
public class JsonSchemaValidatorUtilTest {
    /**
     * 用例场景：测试生成JSONObject
     * 前置条件：bean不为空
     * 检查点：转为org.json.JSONObject成功
     */
    @Test
    public void generate_json_object_success() throws Exception {
        JsonTestFile jsonTestFile = new JsonTestFile("test", "age", 1);
        Object object = Whitebox.invokeMethod(JsonSchemaValidatorUtil.class, "generateJsonObject", jsonTestFile);
        Assert.assertTrue(object instanceof JSONObject);
        Assert.assertEquals("test", ((JSONObject) object).get("name"));
    }

    /**
     * 用例场景：测试生成org.everit.json.schema.Schema
     * 前置条件：schema文件存在
     * 检查点：生成成功
     */
    @Test
    public void generate_json_schema_success() throws Exception {
        String fileName = "test.json";
        Path path = Paths.get("", fileName);
        Optional<Schema> schema = Optional.empty();
        try(InputStream inputStream = getClass().getClassLoader().getResourceAsStream(path.toString())) {
            schema = Whitebox.invokeMethod(JsonSchemaValidatorUtil.class, "generateSchema", fileName, inputStream,
                true);
        }
        Assert.assertTrue(schema.isPresent());
    }

    /**
     * 用例场景：校验bean是否符合Schema规则
     * 前置条件：schema文件存在，bean处在
     * 检查点：校验成功
     */
    @Test
    public void validate_json_bean_success() {
        JsonTestFile jsonTestFile = new JsonTestFile("name", "age", 1);
        String fileName = "test.json";
        JsonSchemaValidatorUtil.validate(jsonTestFile, new String[]{fileName});
    }

    /**
     * 用例场景：校验bean是否符合Schema规则
     * 前置条件：schema文件存在，bean不符合schema规则
     * 检查点：校验成功
     */
    @Test
    public void should_throw_LegoCheckedException_when_validate_json_bean_failed() throws Exception {
        String fileName = "test.json";
        Path path = Paths.get("", fileName);
        try (InputStream inputStream = getClass().getClassLoader().getResourceAsStream(path.toString())) {
            Assert.assertThrows(LegoCheckedException.class,
                () -> JsonSchemaValidatorUtil.validate("", fileName, inputStream, true));
        }
    }

    /**
     * 用例场景：校验bean是否符合Schema规则
     * 前置条件：schema文件不存在
     * 检查点：校验成功
     */
    @Test
    public void should_throw_LegoCheckedException_when_schema_file_is_not_exist() throws Exception {
        JsonTestFile jsonTestFile = new JsonTestFile("name", "age", 1);
        Assert.assertThrows(LegoCheckedException.class,
            () -> JsonSchemaValidatorUtil.validate(jsonTestFile, "fileName", null, true));
    }

    /**
     * 用例场景：Mysql实例注册场景
     * 前置条件：Mysql schema文件存在
     * 检查点：校验成功
     */
    @Test
    public void should_throw_LegoCheckedException_register_mysql_instance_param_error() throws Exception {
        File file = ResourceUtils.getFile("classpath:jsonschema/Mysql-register-test.json");
        String content = FileUtils.readFileToString(file, StandardCharsets.UTF_8);
        String fileName = "jsonschema/MySQL-instance-test_define.json";
        Path path = Paths.get("", fileName);
        try (InputStream inputStream = getClass().getClassLoader().getResourceAsStream(path.toString())) {
            LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> JsonSchemaValidatorUtil.validate(content, fileName, inputStream, true));
            Assert.assertEquals(
                " ErrorKey: #/auth/authKey, ErrorMessage: expected type: String, found: Integer; "
                    + "ErrorKey: #/auth/authKey, ErrorMessage: expected: null, found: Integer; "
                    + "ErrorKey: #/auth/authType, ErrorMessage: expected type: Integer, found: String; "
                    + "ErrorKey: #/auth/extendInfo/instancePort, ErrorMessage: expected type: String, found: Integer; "
                    + "ErrorKey: #/extendInfo, ErrorMessage: required key [linkStatus] not found",
                exception.getMessage());
        }
    }

    /**
     * JsonTestFile
     *
     */
    public static class JsonTestFile {
        private String name;
        private String age;
        private int count;
        public JsonTestFile(String name, String age, int count) {
            this.name = name;
            this.age = age;
            this.count = count;
        }
        public JsonTestFile() {}
        public String getName() {
            return name;
        }
        public void setName(String name) {
            this.name = name;
        }
        public String getAge() {
            return age;
        }
        public void setAge(String age) {
            this.age = age;
        }
        public int getCount() {
            return count;
        }
        public void setCount(int count) {
            this.count = count;
        }
    }
}
