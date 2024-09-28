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
package openbackup.data.access.framework.core.security;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.security.Evaluation;
import openbackup.system.base.security.callee.Callee;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.invocation.InvocationOnMock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.boot.test.mock.mockito.MockBeans;
import org.springframework.context.ApplicationContext;
import org.springframework.test.context.junit4.SpringRunner;

import java.lang.reflect.InvocationTargetException;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

/**
 * Evaluation Test
 *
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@MockBeans({@MockBean(name = "callee_1", classes = Callee.class)})
public class EvaluationTest {
    @Autowired
    private ApplicationContext context;

    @Autowired
    private Callee callee;

    @Test
    public void test_return_detect() {
        test_has_return("a. $return.key", false, false);
        test_has_return("$1.length()", false, false);
        test_has_return(" $return.key", false, true);
        test_has_return("$return .key", false, true);
        test_has_return("T(java.lang.String).valueOf($return.keySet().size())", false, true);
        test_has_return("T(java.lang.String).valueOf(123, $return.keySet().size())", false, true);
        test_has_return("T(java.lang.String).valueOf(T(java.lang.String).valueOf($return.keySet().size()))", false,
            true);
        test_has_return("one( $return.![id])", false, true);

        test_has_return("=prefix#{one( $return.![id])}suffix", false, true);
        test_has_return("prefix#{one( $return.![id])}suffix", true, true);
        test_has_return("prefix#{one( $return.![id])}", true, true);
        test_has_return("#{one( $return.![id])}suffix", true, true);
        test_has_return("#{one( $return.![id])}", true, true);
        test_has_return("$return#{one( array.![id])}", true, false);
    }

    private void test_has_return(String statement, boolean template, boolean expect) {
        Evaluation evaluation = new Evaluation(null, statement, template);
        boolean actual = evaluation.isDependOnReturnValue();
        Assert.assertEquals(expect, actual);
    }

    @Test
    public void test_callee() throws InvocationTargetException, IllegalAccessException {
        PowerMockito.when(callee.call(any())).thenAnswer(InvocationOnMock::getArguments);
        Evaluation evaluation = new Evaluation(context, "@callee_1.call(1,2,3)");
        Object result0 = evaluation.evaluate(Collections::emptyMap);
        Assert.assertArrayEquals(new Object[] {1, 2, 3}, (Object[]) result0);

        Object result1 = new Evaluation(context, "#{@callee_1.call(1,2,3)}", true).evaluate(Collections::emptyMap);
        Assert.assertEquals("1,2,3", result1);
    }

    @Test
    public void test_get_required_variables() {
        test_get_required_variables("$x.y.call($0, $1, $h($x))", false, Arrays.asList("$x", "$0", "$1"));
        test_get_required_variables("$f($0, $1, $h($x))", false, Arrays.asList("$x", "$0", "$1"));
        test_get_required_variables("#one(#list($0, $1).![$2.cast($prefix + #this.name)])", false,
            Arrays.asList("$0", "$1", "$2", "$prefix"));
        test_get_required_variables("$0 < 0 ? $1 : $2", false, Arrays.asList("$0", "$1", "$2"));
        test_get_required_variables("$0?.call($1, $2)", false, Arrays.asList("$0", "$1", "$2"));
        test_get_required_variables("$0?.call($1?:$2)", false, Arrays.asList("$0", "$1", "$2"));
    }

    private void test_get_required_variables(String statement, boolean template, List<String> expect) {
        Evaluation evaluation = new Evaluation(context, statement, template);
        List<String> actual = evaluation.getVariables();
        Assert.assertEquals(new HashSet<>(expect), new HashSet<>(actual));
    }
}
