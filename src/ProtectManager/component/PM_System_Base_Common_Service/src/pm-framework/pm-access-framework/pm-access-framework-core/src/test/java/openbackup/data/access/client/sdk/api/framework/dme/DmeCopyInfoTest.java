/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.beans.IntrospectionException;
import java.lang.reflect.InvocationTargetException;

/**
 * DmeCopyInfo LLT
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/4/20
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({DmeCopyInfo.class})
public class DmeCopyInfoTest extends DmeTest {

    @Before
    public void init() throws IntrospectionException, InvocationTargetException, InstantiationException, IllegalAccessException {
        this.getAndSetTest();
    }

    /**
     * 用例场景：根据dme副本中的校验文件获取副本校验状态
     * 前置条件：
     * 检查点：获取的值不为空
     */
    @Test
    public void should_copy_verify_status_enum() {
        DmeCopyInfo dmeCopyInfo = new DmeCopyInfo();
        dmeCopyInfo.equals("test");
        dmeCopyInfo.hashCode();
        dmeCopyInfo.toString();
        CopyVerifyStatusEnum copyVerifyStatus = dmeCopyInfo.getCopyVerifyStatus();
        Assert.assertNotNull(copyVerifyStatus);
    }

    @Override
    protected DmeCopyInfo getT() {
        return new DmeCopyInfo();
    }
}
