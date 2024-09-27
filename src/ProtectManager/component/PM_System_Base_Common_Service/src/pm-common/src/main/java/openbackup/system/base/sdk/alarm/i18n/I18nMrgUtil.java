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

/**
 * 国际化处理类
 *
 * @author l90005176
 * @version [LEGO V100R002C01, 2011-12-12]
 * @since 2018-01-01
 */
public class I18nMrgUtil {
    private static I18nMrgUtil i18nMrg;

    private I18nMrg i18nMgr = null;

    /**
     * 类实例
     *
     * @return I18nMrgUtil
     */
    public static synchronized I18nMrgUtil getInstance() {
        if (i18nMrg == null) {
            i18nMrg = new I18nMrgUtil();
        }
        return i18nMrg;
    }

    public I18nMrg getI18nMgr() {
        return i18nMgr;
    }

    public void setI18nMgr(I18nMrg i18nMgr) {
        this.i18nMgr = i18nMgr;
    }
}
