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
package openbackup.system.base.sdk.alarm.i18n;

import java.util.Locale;

/**
 * 后台国际化资源文件处理，该类接口暂时只能在PM_System_Base_Common_Service微服务使用，
 * 若要使用该类接口，需要将GUI的国际化文件复制到自己的微服务中，
 * 可参考PM_System_Base_Common_Service如何实现
 *
 */
public interface I18nMrg {
    /**
     * 取得资源文件
     *
     * @param key   资源key值
     * @param local 本地化语言
     * @return String 返回值
     */
    String getString(String key, Locale local);

    /**
     * 取得资源文件，默认取英文
     *
     * @param key 资源key值
     * @return String 返回值
     */
    String getString(String key);

    /**
     * 取得资源文件
     *
     * @param key   资源key值
     * @param local 本地化语言
     * @param agrs  参数
     * @return String 返回值
     */
    String getString(String key, Locale local, String[] agrs);

    /**
     * 获取dorado v6的国际化后参数
     *
     * @param alarmId 资源key值
     * @param local 本地化语言
     * @param args 参数
     * @return 返回值
     */
    String[] getDoradoV6Args(String alarmId, Locale local, String[] args);
}
