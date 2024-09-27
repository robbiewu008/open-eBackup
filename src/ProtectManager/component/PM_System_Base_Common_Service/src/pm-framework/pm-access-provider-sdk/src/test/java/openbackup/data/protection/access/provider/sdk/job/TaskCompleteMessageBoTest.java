/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.job;

import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;

import org.apache.commons.collections.MapUtils;
import org.junit.Assert;
import org.junit.Test;

import java.util.HashMap;
import java.util.Map;

/**
 * TaskCompleteMessageBo类测试用例集合
 *
 * @author: y00559272
 * @version: [OceanProtect X8000 1.2.1]
 * @since: 2022/8/8
 **/
public class TaskCompleteMessageBoTest {

    /**
     * 用例名称：验证扩展参数为空时，返回值为false<br/>
     * 前置条件：无<br/>
     * check点：返回值为false<br/>
     */
    @Test
    public void should_return_false_when_getBooleanFromExtendsInfo_given_extends_info_is_empty() {
        // given
        TaskCompleteMessageBo messageBo = new TaskCompleteMessageBo();
        messageBo.setExtendsInfo(MapUtils.EMPTY_MAP);
        // when
        boolean value = messageBo.getBooleanFromExtendsInfo(
            TaskCompleteMessageBo.ExtendsInfoKeys.IS_DAMAGED);
        // then
        Assert.assertFalse(value);
    }

    /**
     * 用例名称：验证扩展参数为空map时，返回值为false<br/>
     * 前置条件：无<br/>
     * check点：返回值为false<br/>
     */
    @Test
    public void should_return_false_when_getBooleanFromExtendsInfo_given_extends_info_is_null() {
        // given
        TaskCompleteMessageBo messageBo = new TaskCompleteMessageBo();
        // when
        boolean value = messageBo.getBooleanFromExtendsInfo(
            TaskCompleteMessageBo.ExtendsInfoKeys.IS_DAMAGED);
        // then
        Assert.assertFalse(value);
    }

    /**
     * 用例名称：验证扩展参数中value不是字符串时，返回值为false<br/>
     * 前置条件：无<br/>
     * check点：返回值为false<br/>
     */
    @Test
    public void should_return_false_when_getBooleanFromExtendsInfo_given_value_is_not_boolean() {
        // given
        String isDamaged = TaskCompleteMessageBo.ExtendsInfoKeys.IS_DAMAGED;
        TaskCompleteMessageBo messageBo = new TaskCompleteMessageBo();
        Map extendInfo = new HashMap<>();
        extendInfo.put(isDamaged, 128);
        messageBo.setExtendsInfo(extendInfo);
        // when
        boolean value = messageBo.getBooleanFromExtendsInfo(isDamaged);
        // then
        Assert.assertFalse(value);
    }

    /**
     * 用例名称：验证扩展参数中value不是"true"时，返回false<br/>
     * 前置条件：无<br/>
     * check点：返回值为false<br/>
     */
    @Test
    public void should_return_false_when_getBooleanFromExtendsInfo_given_value_is_not_boolean_string() {
        // given
        String isDamaged = TaskCompleteMessageBo.ExtendsInfoKeys.IS_DAMAGED;
        TaskCompleteMessageBo messageBo = new TaskCompleteMessageBo();
        Map extendInfo = new HashMap<>();
        extendInfo.put(isDamaged, "22222");
        messageBo.setExtendsInfo(extendInfo);
        // when
        boolean value = messageBo.getBooleanFromExtendsInfo(isDamaged);
        // then
        Assert.assertFalse(value);
    }

    /**
     * 用例名称：验证扩展参数中value是"true"时，返回true<br/>
     * 前置条件：无<br/>
     * check点：返回值为true<br/>
     */
    @Test
    public void should_return_true_when_getBooleanFromExtendsInfo_given_value_is_true() {
        // given
        String isDamaged = TaskCompleteMessageBo.ExtendsInfoKeys.IS_DAMAGED;
        TaskCompleteMessageBo messageBo = new TaskCompleteMessageBo();
        Map extendInfo = new HashMap<>();
        extendInfo.put(isDamaged, "true");
        messageBo.setExtendsInfo(extendInfo);
        // when
        boolean value = messageBo.getBooleanFromExtendsInfo(isDamaged);
        // then
        Assert.assertTrue(value);
    }
}