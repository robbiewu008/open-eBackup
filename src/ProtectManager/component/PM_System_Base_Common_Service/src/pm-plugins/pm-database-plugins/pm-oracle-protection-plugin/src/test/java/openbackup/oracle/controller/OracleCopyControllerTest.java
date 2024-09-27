package openbackup.oracle.controller;

import openbackup.oracle.controller.OracleCopyController;
import openbackup.oracle.service.OracleCopyService;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2024-01-16
 */
@RunWith(PowerMockRunner.class)
public class OracleCopyControllerTest {
    @InjectMocks
    private OracleCopyController oracleCopyController;

    @Mock
    private OracleCopyService oracleCopyService;

    @Test
    public void test_query_copy_by_scn(){
        oracleCopyController.queryCopyByScn("testResourcId","test");
    }
}

