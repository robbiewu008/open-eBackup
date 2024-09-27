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
package openbackup.data.access.framework.core.security.context;

import openbackup.data.access.framework.core.security.journal.ContextDataRegistration;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.security.context.Context;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Context Registration Test
 *
 * @author l00272247
 * @since 2021-12-15
 */
@RunWith(PowerMockRunner.class)
public class ContextDataRegistrationTest {
    @Rule
    public ExpectedException exceptionRule = ExpectedException.none();

    @Test
    public void test_sort_context_registrations() {
        test_check_circle_reference(new String[] {"$a", "$b", "$c"}, createContext("b", "$a"), createContext("c", "$b"),
            createContext("a", "1"));
        test_check_circle_reference(new String[] {"$y", "$x", "$z"}, createContext("z", "$x"), createContext("x", "$y"),
            createContext("y", "1"));
        test_check_circle_reference(new String[] {"$y", "$x", "$z"}, createContext("z", "$x"), createContext("x", "$y"),
            createContext("y", "1"));
    }

    @Test
    public void test_check_self_reference() {
        exceptionRule.expect(LegoCheckedException.class);
        exceptionRule.expectMessage("self-reference is not allowed");
        createContextRegistrations(createContext("a", "#call($a + 1,$b)"));
    }

    @Test
    public void test_check_name_check() {
        exceptionRule.expect(LegoCheckedException.class);
        exceptionRule.expectMessage("name can not be");
        createContextRegistrations(createContext("", "#call($a + 1,$b)"));
        createContextRegistrations(createContext("return", "#call($a + 1,$b)"));
    }

    @Test
    public void test_check_circle_reference() {
        exceptionRule.expect(LegoCheckedException.class);
        exceptionRule.expectMessage("circle-reference is not allowed");
        createContextRegistrations(createContext("a", "$b"), createContext("b", "$c"), createContext("c", "$a"));
    }

    private void test_check_circle_reference(String[] orders, Context... contexts) {
        List<ContextDataRegistration> registrations = createContextRegistrations(contexts);
        List<String> names = registrations.stream().map(ContextDataRegistration::getName).collect(Collectors.toList());
        Assert.assertEquals(Arrays.asList(orders), names);
    }

    private List<ContextDataRegistration> createContextRegistrations(Context... contexts) {
        List<ContextDataRegistration> registrations = new ArrayList<>();
        return Arrays.stream(contexts)
            .map(context -> new ContextDataRegistration(null, context, registrations))
            .sorted()
            .collect(Collectors.toList());
    }

    private Context createContext(String name, String statement) {
        Context context = PowerMockito.mock(Context.class);
        PowerMockito.when(context.name()).thenReturn(name);
        PowerMockito.when(context.statement()).thenReturn(statement);
        return context;
    }
}
