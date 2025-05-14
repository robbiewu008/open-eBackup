package com.huawei.oceanprotect.system.base.converter;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * 功能描述
 *
 * @author x30046484
 * @since 2024-01-20
 */

@RunWith(PowerMockRunner.class)
public class ConverterUtilTest {
    @Test
    public void test_mutina(){
        System.out.println(ConverterUtil.mutiNa(3));
        Assert.assertEquals("--,--,--",ConverterUtil.mutiNa(3));
    }

    @Test
    public void test_correct_ip_type(){
        Assert.assertTrue(ConverterUtil.correctIptype("ipv4"));
    }

}
