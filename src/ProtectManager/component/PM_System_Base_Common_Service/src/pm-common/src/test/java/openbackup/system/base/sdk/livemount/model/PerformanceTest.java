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
package openbackup.system.base.sdk.livemount.model;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.livemount.model.Performance;

import org.junit.jupiter.api.Assertions;
import org.junit.jupiter.params.ParameterizedTest;
import org.junit.jupiter.params.provider.Arguments;
import org.junit.jupiter.params.provider.MethodSource;

import java.util.stream.Stream;

/**
 * Performance Test
 *
 */
public class PerformanceTest {
    private static Performance create(JSONObject data) {
        return data.toBean(Performance.class);
    }

    private static Stream<Arguments> provide_performances() {
        return Stream.of(Arguments.of(create(new JSONObject()), true),
            Arguments.of(create(new JSONObject().set("min_bandwidth", 1).set("max_bandwidth", 1)), true),
            Arguments.of(create(new JSONObject().set("min_bandwidth", 1).set("max_bandwidth", 2)), true),
            Arguments.of(create(new JSONObject().set("min_bandwidth", 2).set("max_bandwidth", 1)), false),
            Arguments.of(create(new JSONObject().set("min_bandwidth", 1).set("max_bandwidth", 999999999)), true),
            Arguments.of(create(new JSONObject().set("min_bandwidth", 1)), true),
            Arguments.of(create(new JSONObject().set("burst_bandwidth", 1)), false),
            Arguments.of(create(new JSONObject().set("burst_bandwidth", 1).set("max_bandwidth", 2)), false),
            Arguments.of(create(new JSONObject().set("burst_bandwidth", 1).set("max_bandwidth", 1)), false),
            Arguments.of(create(new JSONObject().set("burst_bandwidth", 2).set("max_bandwidth", 1)), false),
            Arguments.of(
                create(new JSONObject().set("burst_bandwidth", 2).set("max_bandwidth", 1).set("burst_time", 1)), true),
            Arguments.of(create(new JSONObject().set("min_iops", 1).set("max_iops", 1)), false),
            Arguments.of(create(new JSONObject().set("min_iops", 100).set("max_iops", 100)), true),
            Arguments.of(create(new JSONObject().set("min_iops", 100).set("max_iops", 1000)), true),
            Arguments.of(create(new JSONObject().set("min_iops", 100).set("max_iops", 1000).set("burst_iops", 1)),
                false),
            Arguments.of(create(new JSONObject().set("min_iops", 100).set("max_iops", 1000).set("burst_iops", 100)),
                false),
            Arguments
                .of(create(new JSONObject().set("min_iops", 100).set("max_iops", 1000).set("burst_iops", 1001)), false),
            Arguments.of(create(new JSONObject().set("burst_iops", 1001)), false),
            Arguments.of(create(new JSONObject().set("min_iops", 100)
                .set("max_iops", 1000)
                .set("burst_iops", 1001)
                .set("burst_time", 1)), true),
            Arguments.of(create(new JSONObject().set("min_iops", 100)
                .set("max_iops", 1000)
                .set("burst_iops", 1000)
                .set("burst_time", 1)), false),
            Arguments.of(create(new JSONObject().set("latency", 1)), false),
            Arguments.of(create(new JSONObject().set("latency", 500)), true),
            Arguments.of(create(new JSONObject().set("latency", 1500)), true));

    }

    /**
     * 用例场景： 验证QoS参数检查逻辑是否正确<br/>
     * 前置条件： QoS参数准备就绪<br/>
     * 检查 点： 检查结果满足驱动数据的预期结果<br/>
     */
    @ParameterizedTest
    @MethodSource("provide_performances")
    public void test_validate(Performance performance, boolean valid) {
        Assertions.assertEquals(valid, performance.validate());
    }
}
