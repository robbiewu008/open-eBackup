package openbackup.system.base.common.aspect;

import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.service.DeployTypeService;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.aop.aspectj.annotation.AnnotationAwareAspectJAutoProxyCreator;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Import;
import org.springframework.test.context.junit4.SpringRunner;

import javax.annotation.Resource;

/**
 * Date Converter
 *
 * @author c30044692
 * @since 2023/4/20
 */
@RunWith(SpringRunner.class)
@Import(AnnotationAwareAspectJAutoProxyCreator.class)
@SpringBootTest(classes = {VerifyDeployTypeAspect.class, VerifyDeployTypeAspectOperation.class})
public class VerifyDeployTypeAspectTest {

    @MockBean
    private DeployTypeService deployTypeService;

    @Resource
    private VerifyDeployTypeAspectOperation verifyDeployTypeAspectOperation;

    /**
     * 测试场景：当部署场景是x3000时，是否能正常处理 <br/>
     * 前置条件：X3000 <br/>
     * 检查点：抛出LegoCheckedException异常
     */
    @Test
    public void Test(){
        Mockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X3000);
        Assert.assertThrows(LegoCheckedException.class, () -> verifyDeployTypeAspectOperation.test(""));
    }


}