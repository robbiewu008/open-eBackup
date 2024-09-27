/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.tidb.resources.access.util.HashUtil;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 *  HashUtil测试类
 *
 * @author w00426202
 * @since 2023-10-12
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(HashUtil.class)
@Slf4j
public class HashUtilTest {
    /**
     * 用例场景：测试digest()
     * 检查点：返回的结果符合预期
     */
    @Test
    public void test_digest() {
        String digestBytes = HashUtil.digest("plain");
        log.info(digestBytes);
        Assert.assertEquals(digestBytes, "a116c9ed46d6207734a43317d30fd88f52ac8634c37d904bbf4e41d865f90475");
    }
}