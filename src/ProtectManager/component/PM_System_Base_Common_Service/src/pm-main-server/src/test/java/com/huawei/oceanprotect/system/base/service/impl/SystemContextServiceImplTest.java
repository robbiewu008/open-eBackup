package com.huawei.oceanprotect.system.base.service.impl;

import static com.huawei.oceanprotect.system.base.initialize.network.enums.InstallationLanguageType.CHINA_LANGUAGE;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.given;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.utils.JSONObject;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigLanguage;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.DeviceManagerService;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import openbackup.system.base.sdk.system.model.TimeZoneInfo;
import com.huawei.oceanprotect.system.base.service.SystemContextServiceImpl;

import junit.framework.TestCase;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.List;

/**
 * SystemContextServiceImpl测试类
 *
 * @author z00445440
 * @since 2023-03-10
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(JSONObject.class)
public class SystemContextServiceImplTest extends TestCase {
    @InjectMocks
    private SystemContextServiceImpl systemContextService;

    @Mock
    private LocalStorageService localStorageService;

    @Mock
    private InitNetworkConfigMapper initNetworkConfigMapper;

    /**
     * 测试用例：获取系统时区
     * 前置条件：无
     * CHECK点：获取系统时区成功。
     */
    @Test
    public void get_system_time_zone_success() {
        TimeZoneInfo timeZoneInfo = new TimeZoneInfo();
        timeZoneInfo.setCmoSysTimeZoneName("beijing");
        List<TimeZoneInfo> timeZoneInfos = new ArrayList<>();
        timeZoneInfos.add(timeZoneInfo);
        DeviceManagerResponse<List<TimeZoneInfo>> deviceManagerResponse = new DeviceManagerResponse<>();
        deviceManagerResponse.setData(timeZoneInfos);
        DeviceManagerService service = Mockito.mock(DeviceManagerService.class);
        given(service.getTimeZoneInfo()).willReturn(deviceManagerResponse);
        given(localStorageService.getLocalStorageDeviceManageService()).willReturn(service);
        TimeZoneInfo timeZone = systemContextService.getSystemTimeZone();
        Assert.assertEquals("beijing", timeZone.getCmoSysTimeZoneName());
        DeviceManagerService serviceFail = Mockito.mock(DeviceManagerService.class);
        PowerMockito.when(localStorageService.getLocalStorageDeviceManageService()).thenReturn(
            serviceFail
        );
        given(serviceFail.getTimeZoneInfo()).willReturn(null);
    }

    @Test
    public void get_system_language_success() {
        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(any())).thenReturn(
            null
        );
        Assert.assertEquals("",systemContextService.getSystemLanguage() );

        PowerMockito.when(initNetworkConfigMapper.queryInitConfig(any())).thenReturn(
            new ArrayList<InitConfigInfo>(){{
                add(new InitConfigInfo(){{
                    setInitValue("aa");
                }});
            }}
        );

        PowerMockito.mockStatic(JSONObject.class);
        PowerMockito.when(JSONObject.toBean(anyString(),any())).thenReturn(new ConfigLanguage(){{
            setLanguage(CHINA_LANGUAGE);
        }});
        Assert.assertEquals("zh-cn",systemContextService.getSystemLanguage() );
    }
}