package openbackup.system.base.common.aspect;

import openbackup.system.base.common.annotation.VerifyDeployType;
import org.springframework.stereotype.Component;

/**
 * Date Converter
 *
 * @author c30044692
 * @since 2023/4/20
 */
@Component
public class VerifyDeployTypeAspectOperation {

    @VerifyDeployType
    public void test(String param){
        System.out.println("xxx");
    }
}
