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
package openbackup.antdb.protection.access.provider.sla;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.MockitoAnnotations;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {AntDBSlaValidator.class})
@PowerMockIgnore( {"javax.management.*", "javax.net.ssl.*", "jdk.internal.reflect.*"})
public class AntDBSlaValidatorTest {
    @InjectMocks
    private AntDBSlaValidator antDBSlaValidator;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);
    }

    @After
    public void tearDown() throws Exception {
    }

    @Test
    public void test_getConfig1_should_return_not_null_when_condition() throws Exception {

        // run the test
        SlaValidateConfig result = antDBSlaValidator.getConfig();

        // verify the results
        assertNotNull(result);
    }

    @Test
    public void test_applicable_should_return_true_when_condition() throws Exception {

        // run the test
        boolean result = antDBSlaValidator.applicable("string");

        // verify the results
        assertTrue(result);
    }

    @Test(expected = NullPointerException.class)
    public void test_applicable_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBSlaValidator.applicable(null);
    }

    @Test
    public void test_validateSLA_should_void_when_condition() throws Exception {
        // setup
        SlaBase slaBase = new SlaBase();

        // run the test
        antDBSlaValidator.validateSLA(slaBase);

        // verify the results
        assertTrue(true);
    }

    @Test(expected = NullPointerException.class)
    public void test_validateSLA_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {

        // run the test
        antDBSlaValidator.validateSLA(null);
    }
}