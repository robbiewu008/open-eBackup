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
package openbackup.antdb.protection.access.provider.agent;

import static org.junit.jupiter.api.Assertions.assertNotNull;
import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.junit.jupiter.api.Assertions.assertTrue;
import static org.mockito.Mockito.any;
import static org.mockito.Mockito.when;

import openbackup.antdb.protection.access.service.AntDBInstanceService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;
import org.mockito.Answers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.mockito.junit.jupiter.MockitoSettings;
import org.mockito.quality.Strictness;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.ArrayList;
import java.util.List;

@MockitoSettings(strictness = Strictness.LENIENT)
class AntDBAgentProviderTest {
    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private AntDBInstanceService antDBInstanceService;

    @InjectMocks
    private AntDBAgentProvider antDBAgentProvider;

    private AutoCloseable mockitoCloseable;

    @BeforeEach
    void setUp() throws Exception {
        mockitoCloseable = MockitoAnnotations.openMocks(this);
        ReflectionTestUtils.setField(antDBAgentProvider, "antDBInstanceService", antDBInstanceService);
    }

    @AfterEach
    void tearDown() throws Exception {
        mockitoCloseable.close();
    }

    @Test
    void test_getSelectedAgents_should_return_not_null_when_condition() throws Exception {
        // setup
        List<Endpoint> list = new ArrayList<>();
        Endpoint endpoint = new Endpoint();
        list.add(endpoint);
        when(antDBInstanceService.getAgentsByInstanceResource(any(ProtectedResource.class))).thenReturn(list);

        ProtectedResource resource = new ProtectedResource();
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(resource).build();

        // run the test
        List<Endpoint> result = antDBAgentProvider.getSelectedAgents(agentSelectParam);

        // verify the results
        assertNotNull(result);
    }

    @Test
    void test_getSelectedAgents_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        assertThrows(NullPointerException.class, () -> {
            // setup
            when(antDBInstanceService.getAgentsByInstanceResource(any(ProtectedResource.class))).thenReturn(null);

            // run the test
            List<Endpoint> result = antDBAgentProvider.getSelectedAgents((AgentSelectParam) null);
        });
    }

    @Test
    void test_getSelectedAgents_should_return_not_null_when_objects_is_null() throws Exception {
        // setup
        when(antDBInstanceService.getAgentsByInstanceResource(any(ProtectedResource.class))).thenReturn(null);

        // run the test
        List<Endpoint> result = antDBAgentProvider.getSelectedAgents((AgentSelectParam) null);

        // verify the results
        assertNotNull(result);
    }

    @Test
    void test_applicable_should_return_true_when_condition() throws Exception {
        // setup
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType("string");
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(resource).build();

        // run the test
        boolean result = antDBAgentProvider.applicable(agentSelectParam);

        // verify the results
        assertTrue(result);
    }

    @Test
    void test_applicable_should_throws_null_pointer_exception_when_objects_is_null() throws Exception {
        assertThrows(NullPointerException.class, () -> {

            // run the test
            boolean result = antDBAgentProvider.applicable((AgentSelectParam) null);
        });
    }

    @Test
    void test_applicable_should_return_true_when_objects_is_null() throws Exception {

        // run the test
        boolean result = antDBAgentProvider.applicable((AgentSelectParam) null);

        // verify the results
        assertTrue(result);
    }
}