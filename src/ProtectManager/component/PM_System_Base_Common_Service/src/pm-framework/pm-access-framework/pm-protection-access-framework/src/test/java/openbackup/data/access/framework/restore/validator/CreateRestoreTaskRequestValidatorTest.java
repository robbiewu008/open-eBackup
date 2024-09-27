package openbackup.data.access.framework.restore.validator;

import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.springframework.validation.Errors;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

/**
 * 恢复任务创建请求校验器测试类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/3/8
 **/
public class CreateRestoreTaskRequestValidatorTest {
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);
    private final CreateRestoreTaskRequestValidator validator = new CreateRestoreTaskRequestValidator(copyRestApi);

    /**
     * 用例名称：验证校验器类型匹配成功<br/>
     * 前置条件：无<br/>
     * check点：当类为CreateRestoreTaskRequest时匹配false，当类不为CreateRestoreTaskRequest时结果为false<br/>
     */
    @Test
    public void should_return_true_when_supports_given_CreateRestoreTaskRequest_class(){
        Assert.assertTrue(validator.supports(CreateRestoreTaskRequest.class));
        Assert.assertFalse(validator.supports(CreateRestoreTaskRequestValidatorTest.class));
    }

    /**
     * 用例名称：验证校验器类逻辑执行成功<br/>
     * 前置条件：无<br/>
     * check点：业务逻辑正常执行，并且返回的copy信息为指定的信息<br/>
     */
    @Test
    public void should_success_when_customValidate_given_CreateRestoreTaskRequest(){
        // GIVEN
        final CreateRestoreTaskRequest createRestoreTaskRequest = new CreateRestoreTaskRequest();
        final Errors mock = Mockito.mock(Errors.class);
        final Copy copy = CopyMocker.mockCommonCopy();
        given(copyRestApi.queryCopyByID(any())).willReturn(copy);
        // WHEN
        validator.customValidate(createRestoreTaskRequest, mock);
        // THEN
        Assert.assertNotNull(createRestoreTaskRequest.getCopy());
        Assert.assertEquals(createRestoreTaskRequest.getCopy(), copy);
   }
}