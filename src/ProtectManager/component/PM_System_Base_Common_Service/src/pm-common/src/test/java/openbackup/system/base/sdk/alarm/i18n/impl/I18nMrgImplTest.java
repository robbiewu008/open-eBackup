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
package openbackup.system.base.sdk.alarm.i18n.impl;

import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.sdk.alarm.i18n.impl.I18nMrgImpl;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.AdapterUtils;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Locale;

/**
 * I18nMrgImplTest
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/4/13
 */
public class I18nMrgImplTest {
    private I18nMrgImpl i18nMrgImpl = new I18nMrgImpl();

    private Method getAllClassPathEntriesMethod;

    private Method processUrlListMethod;

    private Field resMapField;

    private String doradoAlarmFileName = "testDoradoAlarm.json";

    private String doradoPacificFileName = "testPacificAlarm.json";

    @Before
    public void init() {
        try {
            Method[] declaredMethods = I18nMrgImpl.class.getDeclaredMethods();
            for (Method declaredMethod : declaredMethods) {
                declaredMethod.setAccessible(true);
                if (declaredMethod.getName().equals("processUrlList") && declaredMethod.getParameterCount() == 5) {
                    processUrlListMethod = declaredMethod;
                }
                if (declaredMethod.getName().equals("setSelf")) {
                    declaredMethod.invoke(i18nMrgImpl);
                }
            }

            resMapField = I18nMrgImpl.class.getDeclaredField("RES_MAP");
            resMapField.setAccessible(true);

            // 设置DeployTypeService
            DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);
            Mockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X8000);
            Field deployTypeServiceField = I18nMrgImpl.class.getDeclaredField("deployTypeService");
            deployTypeServiceField.setAccessible(true);
            deployTypeServiceField.set(i18nMrgImpl, deployTypeService);

            getAllClassPathEntriesMethod = AdapterUtils.class.getDeclaredMethod("getAllClassPathEntries", String.class,
                String.class);
            getAllClassPathEntriesMethod.setAccessible(true);
        } catch (Exception ignored) {
        }
    }

    /**
     * 用例名称：dorado中文告警能够正常国际化
     * 前置条件：无
     * check点：无报错
     */
    @Test
    public void dorado_alarm_i18n_chinese_success() {
        loadAlarmFile("conf/alarmI18nZ/", doradoAlarmFileName, Locale.CHINESE, null);
        String[] doradoV6Args = i18nMrgImpl.getDoradoV6Args("0x64006E0001", Locale.CHINESE,
            new String[] {"数据库A", "1", "2"});
        String desc = i18nMrgImpl.getString("0x64006E0001.alarm.desc", Locale.CHINESE, doradoV6Args);
        Assert.assertTrue(desc.contains("归档任务成功"));
    }

    /**
     * 用例名称：dorado英文告警能够正常国际化
     * 前置条件：无
     * check点：无报错
     */
    @Test
    public void dorado_alarm_i18n_english_success() {
        loadAlarmFile("conf/alarmI18nE/", doradoAlarmFileName, Locale.ENGLISH, null);
        String[] doradoV6Args = i18nMrgImpl.getDoradoV6Args("0x64006E0001", Locale.ENGLISH,
            new String[] {"Database A", "1", "2"});
        String desc = i18nMrgImpl.getString("0x64006E0001.alarm.desc", Locale.ENGLISH, doradoV6Args);
        Assert.assertTrue(desc.contains("archive succeeded"));
    }

    /**
     * 用例名称：在E6000环境下，E6000告警高于dorado告警
     * 前置条件：无
     * check点：无报错
     */
    @Test
    public void pacific_priority_dorado_alarm() throws NoSuchFieldException, IllegalAccessException {
        // 设置DeployTypeService
        DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);
        Mockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);
        Field deployTypeServiceField = I18nMrgImpl.class.getDeclaredField("deployTypeService");
        deployTypeServiceField.setAccessible(true);
        deployTypeServiceField.set(i18nMrgImpl, deployTypeService);

        loadAlarmFile("conf/alarmI18nZ/", doradoAlarmFileName, Locale.CHINESE, null);
        loadAlarmFile("conf/alarmI18nZ/", doradoPacificFileName, Locale.CHINESE, DeployTypeEnum.E6000);
        String[] doradoV6Args = i18nMrgImpl.getDoradoV6Args("0x64006E0001", Locale.CHINESE,
            new String[] {"数据库A", "1", "2"});
        String desc = i18nMrgImpl.getString("0x64006E0001.alarm.desc", Locale.CHINESE, doradoV6Args);
        Assert.assertTrue(desc.contains("xxx归档任务成功"));
    }

    /**
     * 用例名称：在E6000环境下，英文告警文件中argument.explain为空的情况下也能正常国际化
     * 前置条件：无
     * check点：无报错
     */
    @Test
    public void pacific_english_can_i18n_when_argument_explain_is_empty()
        throws NoSuchFieldException, IllegalAccessException {
        // 设置DeployTypeService
        DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);
        Mockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.E6000);
        Field deployTypeServiceField = I18nMrgImpl.class.getDeclaredField("deployTypeService");
        deployTypeServiceField.setAccessible(true);
        deployTypeServiceField.set(i18nMrgImpl, deployTypeService);

        loadAlarmFile("conf/alarmI18nZ/", doradoPacificFileName, Locale.CHINESE, DeployTypeEnum.E6000);
        loadAlarmFile("conf/alarmI18nE/", doradoPacificFileName, Locale.ENGLISH, DeployTypeEnum.E6000);
        String[] doradoV6Args = i18nMrgImpl.getDoradoV6Args("0xF3C01001C", Locale.ENGLISH, new String[] {"11"});
        String desc = i18nMrgImpl.getString("0xF3C01001C.alarm.desc", Locale.ENGLISH, doradoV6Args);
        Assert.assertTrue(desc.contains("(ID 11)"));
    }

    private void loadAlarmFile(String path, String fileName, Locale locale, DeployTypeEnum deployTypeEnum) {
        Object urls = null;
        try {
            urls = getAllClassPathEntriesMethod.invoke(null, path, fileName);
            processUrlListMethod.invoke(i18nMrgImpl, urls, locale, deployTypeEnum, null, true);
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw new RuntimeException(e);
        }
    }
}
