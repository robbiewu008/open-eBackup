package openbackup.system.base.common.aspect.verifier;

import openbackup.system.base.common.aspect.verifier.CommonOwnershipVerifier;
import openbackup.system.base.common.constants.TokenBo;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;

/**
 * Common Ownership Verifier Test
 *
 * @Since:2023-4-20
 * @Author:c30044692
 */
@RunWith(SpringRunner.class)
public class CommonOwnershipVerifierTest {

    private CommonOwnershipVerifier verifier;

    @Before
    public void create_verifier(){
        verifier = new CommonOwnershipVerifier("test", (p1, p2) -> {});
    }

    /**
     * 测试场景：测试新建CommonOwnershipVerifier后 获取type是否正确 <br/>
     * 前置条件：无 <br/>
     * 检查点：type和预期一致
     */
    @Test
    public void test_get_type() {
        Assert.assertEquals("test", verifier.getType());
    }

    /**
     * 测试场景：当resources为空时，是否正确处理 <br/>
     * 前置条件：resources为空 <br/>
     * 检查点：方法正常返回
     */
    @Test
    public void should_pass_when_resources_is_empty() {
        verifier.verify(new TokenBo.UserBo(), null);
    }

    /**
     * 测试场景：当resources不为空时，是否正确处理 <br/>
     * 前置条件：resources不为空 <br/>
     * 检查点：方法正常返回
     */
    @Test
    public void should_pass_when_resources_is_not_empty() {
        verifier.verify(new TokenBo.UserBo(), Arrays.asList("1","2"));
    }
}
