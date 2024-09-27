package openbackup.access.framework.resource.validator;

import openbackup.access.framework.resource.validator.mock_context.Dimensions;
import openbackup.access.framework.resource.validator.mock_context.NoProductIdProduct;
import openbackup.access.framework.resource.validator.mock_context.Product;

import openbackup.system.base.common.exception.LegoCheckedException;

import openbackup.system.base.common.validator.JsonSchemaValidatorUtil;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.List;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * 功能描述
 *
 * @author w00616953
 * @since 2021-10-14
 */
@SpringBootTest
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@ContextConfiguration(classes = {JsonSchemaValidatorImpl.class})
@PrepareForTest(JsonSchemaValidatorUtil.class)
public class JsonSchemaValidatorTest {
    @Autowired
    private JsonSchemaValidator jsonSchemaValidator;

    private Product product;

    @Before
    public void before() {
        product = new Product();
        product.setProductId(1);
        product.setProductName("An ice sculpture");
        product.setPrice(1);
        product.setTags(new String[] {"cold", "ice"});
        Dimensions dimensions = new Dimensions();
        dimensions.setName("name");
        dimensions.setLength(7.0);
        dimensions.setWidth(12.0);
        dimensions.setHeight(9.5);
        product.setDimensions(dimensions);
    }

    /**
     * 用例场景： 传入的数据格式合法的情况下能够通过校验
     * 前置条件： 存在指定的JsonSchema文件，传入的数据符合模式
     * 检查 点：  在指定的JsonSchema文件存在的情况下，合法的数据能够通过校验
     */
    @Test
    public void test_validate_success() {
        jsonSchemaValidator.doValidate(product, "product_Env");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景： 传入的数据格式合法的情况下能够通过校验
     * 前置条件： 存在指定的JsonSchema文件，传入的数据符合模式
     * 检查 点：  在指定的JsonSchema文件存在的情况下，合法的数据能够通过校验，传入的jsonschema名称匹配第一个
     */
    @Test
    public void should_validate_success_when_has_schema_file_and_first_is_match() throws Exception {
        PowerMockito.mockStatic(JsonSchemaValidatorUtil.class);
        jsonSchemaValidator.doValidate(product, new String[]{"product_Env", "protected_environment_Env"});
        PowerMockito.verifyStatic(JsonSchemaValidatorUtil.class, Mockito.times(1));
        JsonSchemaValidatorUtil.validate(any(), any(), anyString());
    }

    /**
     * 用例场景： 传入的数据格式合法的情况下能够通过校验
     * 前置条件： 存在指定的JsonSchema文件，传入的数据符合模式
     * 检查 点：  在指定的JsonSchema文件存在的情况下，合法的数据能够通过校验，传入的jsonschema名称匹配第二个
     */
    @Test
    public void should_validate_success_when_has_schema_file_and_second_is_match() throws Exception {
        PowerMockito.mockStatic(JsonSchemaValidatorUtil.class);
        jsonSchemaValidator.doValidate(product, new String[]{"fake_Env", "product_Env", "protected_environment_Env"});
        PowerMockito.verifyStatic(JsonSchemaValidatorUtil.class, Mockito.times(1));
        JsonSchemaValidatorUtil.validate(any(), any(), anyString());;
    }


    /**
     * 用例场景： 如果指定的JsonSchema文件不存在，获取可编辑或机密字段时直接返回空列表
     * 前置条件： 不存在指定的JsonSchema文件
     * 检查 点：  不强制要求JsonSchema文件存在，不存在则返回空列表
     */
    @Test
    public void test_should_return_empty_list_when_json_schema_file_does_not_exist() {
        List<String> editableFields = jsonSchemaValidator.getEditableFields("product");
        Assert.assertEquals(0, editableFields.size());

        List<String> secretFields = jsonSchemaValidator.getSecretFields("product");
        Assert.assertEquals(0, secretFields.size());
    }

    /**
     * 用例场景： 传入的数据格式不合法，校验不通过
     * 前置条件： 存在指定的JsonSchema文件，传入的数据不符合模式
     * 检查 点：  在指定的JsonSchema文件存在的情况下，不合法的数据不能够通过校验
     */
    @Test
    public void test_should_raise_LegoCheckedException_when_number_out_of_range() {
        try {
            product.setPrice(-1);
            jsonSchemaValidator.doValidate(product, "product_Env");
        } catch (LegoCheckedException e) {
            Assert.assertEquals(1677929220, e.getErrorCode());
            Assert.assertEquals(
                    " ErrorKey: #/price, ErrorMessage: -1.0 is not greater or equal to 0",
                    e.getMessage());
        }
    }

    /**
     * 用例场景： 传入的数据格式不合法，校验不通过
     * 前置条件： 存在指定的JsonSchema文件，传入的数据不符合模式
     * 检查 点：  在指定的JsonSchema文件存在的情况下，不合法的数据不能够通过校验
     */
    @Test
    public void test_should_raise_LegoCheckedException_when_required_element_not_exist() {
        try {
            NoProductIdProduct noProductIdProduct = new NoProductIdProduct();
            noProductIdProduct.setPrice(1);
            noProductIdProduct.setProductName("An ice sculpture");
            noProductIdProduct.setTags(new String[] {"cold", "ice"});
            Dimensions dimensions = new Dimensions();
            dimensions.setLength(7.0);
            dimensions.setWidth(12.0);
            dimensions.setHeight(9.5);
            noProductIdProduct.setDimensions(dimensions);
            jsonSchemaValidator.doValidate(noProductIdProduct, "product_Env");
        } catch (LegoCheckedException e) {
            Assert.assertEquals(1677929220, e.getErrorCode());
            Assert.assertEquals(
                " ErrorKey: #, ErrorMessage: required key [productId] not found; ErrorKey: #/dimensions/name, "
                    + "ErrorMessage: expected type: String, found: Null",
                e.getMessage());
        }
    }

    /**
     * 用例场景： 如果JsonSchema不指定字段属性可为null，传入null时会抛出异常
     * 前置条件： 存在指定的JsonSchema文件，传入的数据不符合模式
     * 检查 点：  传入的数据格式不合法，校验不通过
     */
    @Test
    public void test_should_raise_LegoCheckedException_when_validate_if_field_is_null_but_specified_string() {
        try {
            Product noProductIdProduct = new Product();
            product.setProductId(1);
            noProductIdProduct.setPrice(1);
            noProductIdProduct.setTags(new String[] {"cold", "ice"});
            noProductIdProduct.setProductName("An ice sculpture");
            Dimensions dimensions = new Dimensions();
            dimensions.setName(null);
            dimensions.setLength(7);
            dimensions.setWidth(12);
            dimensions.setHeight(9);
            noProductIdProduct.setDimensions(dimensions);
            jsonSchemaValidator.doValidate(noProductIdProduct, "product_Env");
        } catch (LegoCheckedException e) {
            Assert.assertEquals(1677929220, e.getErrorCode());
            Assert.assertEquals(
                    " ErrorKey: #/dimensions/name, ErrorMessage: expected type: String, found: Null",
                    e.getMessage());
        }
    }

    /**
     * 用例场景： 如果JsonSchema指定字段属性可为null，传入null时不会抛出异常
     * 前置条件： 存在指定的JsonSchema文件，传入的数据不符合模式
     * 检查 点：  传入的数据格式合法，校验通过
     */
    @Test
    public void test_should_validate_success_when_validate_if_field_is_null_and_specified_can_be_null() {
        Product noProductIdProduct = new Product();
        product.setProductId(1);
        noProductIdProduct.setPrice(1);
        noProductIdProduct.setTags(new String[] {"cold", "ice"});
        noProductIdProduct.setProductName(null);
        Dimensions dimensions = new Dimensions();
        dimensions.setName("name");
        dimensions.setLength(7);
        dimensions.setWidth(12);
        dimensions.setHeight(9);
        noProductIdProduct.setDimensions(dimensions);
        jsonSchemaValidator.doValidate(noProductIdProduct, "product_Env");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景： 能够成功获取JsonSchema文件中的secret字段
     * 前置条件： 存在指定的JsonSchema文件，且存在secret字段
     * 检查 点：  在指定的JsonSchema文件存在的情况下且存在secret字段的情况下，能够获取到其元素
     */
    @Test
    public void test_secret_fields_not_empty_when_schema_has_secret_keyword() {
        List<String> secretFields = jsonSchemaValidator.getSecretFields("product_Env");
        Assert.assertEquals(4, secretFields.size());
    }

    /**
     * 用例场景： 能够成功获取JsonSchema文件中的editable字段
     * 前置条件： 存在指定的JsonSchema文件，且存在editable字段
     * 检查 点：  在指定的JsonSchema文件存在的情况下且存在editable字段的情况下，能够获取到其元素
     */
    @Test
    public void test_editable_fields_not_empty_when_schema_has_editable_keyword() {
        List<String> editableFields = jsonSchemaValidator.getEditableFields("product_Env");
        Assert.assertEquals(5, editableFields.size());
    }
}
