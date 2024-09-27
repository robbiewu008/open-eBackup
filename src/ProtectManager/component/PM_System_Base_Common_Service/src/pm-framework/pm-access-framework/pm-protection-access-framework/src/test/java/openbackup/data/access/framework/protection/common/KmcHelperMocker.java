/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.protection.common;

import com.huawei.emeistor.kms.kmc.util.KmcHelper;

import org.junit.Before;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;

/**
 * KmcHelper的公共模拟器，其它的类继承该类。
 * 继承该类时：
 * 1.必须使用PowerMockRunner执行测试
 * 2.必须在@PrepareForTest中声明KmcHelper.class
 *
 * @author y00559272
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/3/24
 **/
@PrepareForTest(value = {KmcHelper.class})
public class KmcHelperMocker {

    @Before
    public void classInit() {
        PowerMockito.mockStatic(KmcHelper.class);
        KmcHelper kmcHelper = PowerMockito.mock(KmcHelper.class);
        PowerMockito.when(KmcHelper.getInstance()).thenReturn(kmcHelper);
    }
}
