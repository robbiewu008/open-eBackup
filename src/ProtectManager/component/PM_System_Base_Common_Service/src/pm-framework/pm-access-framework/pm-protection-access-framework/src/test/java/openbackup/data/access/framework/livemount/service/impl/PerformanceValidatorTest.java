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
package openbackup.data.access.framework.livemount.service.impl;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.service.impl.PerformanceValidator;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;
import openbackup.system.base.sdk.livemount.model.Performance;

import org.hibernate.validator.internal.engine.ValidatorFactoryImpl;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;
import java.util.Map;

import javax.validation.Validator;
import javax.validation.ValidatorFactory;

/**
 * PerformanceValidator test
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {PerformanceValidator.class, ValidatorFactoryImpl.class})
public class PerformanceValidatorTest {
    @Autowired
    private PerformanceValidator performanceValidator;

    @MockBean
    private ValidatorFactory factory;

    @Test
    public void validate_performance_success() {
        Validator validator = Mockito.mock(Validator.class);
        Mockito.when(factory.getValidator()).thenReturn(validator);

        Performance performance = new Performance();
        performanceValidator.validatePerformance(performance);
        Mockito.verify(validator, Mockito.times(1)).validate(any());
    }

    @Test
    public void get_valid_performance_succes() {
        Mockito.when(factory.getValidator()).thenReturn(Mockito.mock(Validator.class));

        LiveMountObject liveMountObject = getLiveMountObject();
        Performance validPerformance = performanceValidator.getValidPerformance(liveMountObject);
        Assert.assertNotNull(validPerformance);
    }

    @Test
    public void load_performance_success() {
        Mockito.when(factory.getValidator()).thenReturn(Mockito.mock(Validator.class));

        LiveMountObject liveMountObject = getLiveMountObject();
        liveMountObject.setCopyId("111");
        Map<String, Object> parameters = liveMountObject.getParameters();
        Performance performance = performanceValidator.loadPerformance(parameters);
        Assert.assertNull(performance);
    }

    private LiveMountObject getLiveMountObject() {
        LiveMountObject liveMountObject = new LiveMountObject();
        liveMountObject.setTargetResourceUuidList(Arrays.asList("uuid1", "uuid2"));
        liveMountObject.setSourceResourceId("uuid0");
        liveMountObject.setCopyId("copyId");
        liveMountObject.setTargetLocation(LiveMountTargetLocation.ORIGINAL);
        Performance performance = new Performance();
        Map<String, Object> map = JSONObject.fromObject(performance).toMap(Object.class);
        map.put("test", "copyid-1");
        liveMountObject.setParameters(map);
        return liveMountObject;
    }
}
