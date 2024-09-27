/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.asserts;

import static org.assertj.core.api.Assertions.assertThatThrownBy;

import openbackup.system.base.common.utils.asserts.PowerAssert;

import org.junit.jupiter.api.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * PowerAssert工具类的测试用例集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/21
 **/
public class PowerAssertTest {

    /**
     * 用例名称：验证state可以在条件为false的时候抛出指定异常<br/>
     * 前置条件：无<br/>
     * check点：1. 异常类型一致 2. 异常信息一致<br/>
     */
    @Test
    public void should_throw_given_exception_when_test_state_given_specify_exception() {
        // Given
        String message = "test error";
        PowerAssert.state(Boolean.TRUE, () -> new RuntimeException(message));
        assertThatThrownBy(() -> PowerAssert.state(Boolean.FALSE, () -> new RuntimeException(message))).isInstanceOf(
            RuntimeException.class).hasMessageContaining(message);
    }

    /**
     * 用例名称：验证参数为null的时候抛出指定异常<br/>
     * 前置条件：无<br/>
     * check点：1. 异常类型一致 2. 异常信息一致<br/>
     */
    @Test
    public void should_throw_given_exception_when_test_notNull_given_specify_exception() {
        // Given
        String message = "test error";
        PowerAssert.notNull("1111", () -> new RuntimeException(message));
        assertThatThrownBy(() -> PowerAssert.notNull(null, () -> new RuntimeException(message))).isInstanceOf(
            RuntimeException.class).hasMessageContaining(message);
    }

    /**
     * 用例名称：验证集合类为空的时候抛出指定异常<br/>
     * 前置条件：无<br/>
     * check点：1. 异常类型一致 2. 异常信息一致<br/>
     */
    @Test
    public void should_throw_given_exception_when_test_notEmpty_given_empty_list_specify_exception() {
        // Given
        String message = "test error";
        // When and Then
        PowerAssert.notEmpty(Collections.singletonList("111"), () -> new RuntimeException(message));
        assertThatThrownBy(() -> PowerAssert.notEmpty(new ArrayList<>(), () -> new RuntimeException(message))).isInstanceOf(
            RuntimeException.class).hasMessageContaining(message);
    }

    /**
     * 用例名称：验证集合类为空的时候抛出指定异常<br/>
     * 前置条件：无<br/>
     * check点：1. 异常类型一致 2. 异常信息一致<br/>
     */
    @Test
    public void should_throw_given_exception_when_test_notEmpty_given_empty_map_specify_exception() {
        // Given
        String message = "test error";
        Map<String, String> testMap = new HashMap<>();
        testMap.put("aaaa", "bbbb");
        // When and Then
        PowerAssert.notEmpty(testMap, () -> new RuntimeException(message));
        assertThatThrownBy(() -> PowerAssert.notEmpty(new HashMap<>(), () -> new RuntimeException(message))).isInstanceOf(
            RuntimeException.class).hasMessageContaining(message);
    }

    /**
     * 用例名称：验证字符串为空或者空串的时候抛出指定异常<br/>
     * 前置条件：无<br/>
     * check点：1. 异常类型一致 2. 异常信息一致<br/>
     */
    @Test
    public void should_throw_given_exception_when_test_notBlank_given_specify_exception() {
        // Given
        String message = "test error";
        PowerAssert.notBlank("1111", () -> new RuntimeException(message));
        assertThatThrownBy(() -> PowerAssert.notBlank("", () -> new RuntimeException(message))).isInstanceOf(
            RuntimeException.class).hasMessageContaining(message);
    }
}