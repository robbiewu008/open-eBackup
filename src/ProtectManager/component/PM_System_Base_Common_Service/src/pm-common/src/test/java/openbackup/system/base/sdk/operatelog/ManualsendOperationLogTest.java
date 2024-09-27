/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.operatelog;

import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.sdk.operationlog.ManualSendOperationLogImpl;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import javax.annotation.Resource;

/**
 * manual operate log
 *
 * @author h30003246
 * @since 2021-05-24
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {ManualSendOperationLogImpl.class})
public class ManualsendOperationLogTest {
    @MockBean
    private TokenVerificationService tokenVerificationService;

    @MockBean
    private OperationLogService operationLogService;

    @Resource
    private ManualSendOperationLogImpl manualSendOperationLog;

    /**
     *  send operation log success
     */
    @Test
    public void send_operation_log_is_success() {
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setName("admin");
        userBo.setId("3434456567");
        long exp = System.currentTimeMillis();
        long created = System.currentTimeMillis();
        TokenBo tokenBo = TokenBo.builder().user(userBo).exp(exp).created(created).build();

        PowerMockito.when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(tokenBo);

        PowerMockito.doNothing().when(operationLogService).sendEvent(ArgumentMatchers.any());

        manualSendOperationLog.sendOperationLog("User", "Ox3456554502");
    }

}
