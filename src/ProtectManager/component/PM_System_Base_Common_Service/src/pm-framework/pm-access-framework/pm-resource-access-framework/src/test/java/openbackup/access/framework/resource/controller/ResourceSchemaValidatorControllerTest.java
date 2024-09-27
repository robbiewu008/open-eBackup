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

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;

import openbackup.access.framework.resource.validator.JsonSchemaValidatorImpl;
import openbackup.access.framework.resource.validator.mock_context.Dimensions;
import openbackup.access.framework.resource.validator.mock_context.Product;
import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.http.MediaType;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.MvcResult;
import org.springframework.web.util.NestedServletException;

import java.util.Objects;

/**
 * 功能描述
 *
 * @author w00616953
 * @since 2021-10-20
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {ResourceSchemaValidatorController.class})
@AutoConfigureMockMvc
@AutoConfigureWebMvc
@ContextConfiguration(classes = {JsonSchemaValidatorImpl.class})
public class ResourceSchemaValidatorControllerTest {
    @Autowired
    private MockMvc mockMvc;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    private JsonNode jsonNode;

    @Before
    public void before() {
        ObjectMapper objectMapper = JSONObject.createObjectMapper();
        Product product = new Product();

        product.setProductId(1);
        product.setProductName("An ice sculpture");
        product.setPrice(1);
        product.setTags(new String[] {"cold", "ice"});
        Dimensions dimensions = new Dimensions();
        dimensions.setLength(7.0);
        dimensions.setWidth(12.0);
        dimensions.setHeight(9.5);
        dimensions.setName("test");
        product.setDimensions(dimensions);
        jsonNode = objectMapper.valueToTree(product);
    }

    @Test
    public void test_do_validate_success() throws Exception {
        MvcResult result =
                mockMvc.perform(
                                get("/v2/internal/jsonschemas/{schemaName}/validation/result",
                                        "product_Env")
                                        .content(jsonNode.toPrettyString())
                                        .contentType(MediaType.APPLICATION_JSON))
                        .andReturn();
        Assert.assertEquals(200, result.getResponse().getStatus());
    }

    @Test
    public void test_should_raise_JsonParseException_if_checked_data_can_not_transform_to_json() throws Exception {
        MvcResult result =
                mockMvc.perform(
                                get("/v2/internal/jsonschemas/{schemaName}/validation/result",
                                        "product_Env")
                                        .content("test")
                                        .contentType(MediaType.APPLICATION_JSON))
                        .andReturn();
        Assert.assertEquals(400, result.getResponse().getStatus());
        Assert.assertTrue(
                Objects.requireNonNull(result.getResolvedException()).getMessage().contains("JsonParseException"));
    }

    @Test
    public void test_should_raise_HttpMessageNotReadableException_if_checked_data_is_empty() throws Exception {
        MvcResult result =
                mockMvc.perform(
                                get("/v2/internal/jsonschemas/{schemaName}/validation/result",
                                        "product_Env")
                                        .content("")
                                        .contentType(MediaType.APPLICATION_JSON))
                        .andReturn();
        Assert.assertEquals(400, result.getResponse().getStatus());
        Assert.assertTrue(
                Objects.requireNonNull(result.getResolvedException())
                        .getMessage()
                        .contains("Required request body is missing"));
    }

    @Test
    public void test_should_raise_HttpMessageNotReadableException_if_json_schema_file_does_not_exist()
            throws Exception {
        try {
            mockMvc.perform(
                    get("/v2/internal/jsonschemas/{schemaName}/validation/result",
                            "product")
                            .content(jsonNode.toPrettyString())
                            .contentType(MediaType.APPLICATION_JSON));
        } catch (NestedServletException e) {
            Assert.assertTrue(Objects.requireNonNull(e.getMessage()).contains("The json schema file does not exist."));
        }
    }

    @Test
    public void test_get_editable_fields_success() throws Exception {
        MvcResult rs =
                mockMvc.perform(
                                get("/v2/internal/jsonschemas/{schemaName}/fields/editable",
                                        "product_Env"))
                        .andReturn();
        Assert.assertEquals(
                "[\"productName\",\"price\",\"dimensions.length\",\"dimensions.width\",\"area.dimensions.l\"]",
                rs.getResponse().getContentAsString());
    }

    @Test
    public void test_get_secret_fields_success() throws Exception {
        MvcResult rs =
                mockMvc.perform(
                                get("/v2/internal/jsonschemas/{schemaName}/fields/secret",
                                        "product_Env"))
                        .andReturn();
        Assert.assertEquals(
                "[\"productId\",\"price\",\"dimensions.length\",\"dimensions.height\"]",
                rs.getResponse().getContentAsString());
    }
}
