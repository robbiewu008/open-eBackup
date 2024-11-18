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