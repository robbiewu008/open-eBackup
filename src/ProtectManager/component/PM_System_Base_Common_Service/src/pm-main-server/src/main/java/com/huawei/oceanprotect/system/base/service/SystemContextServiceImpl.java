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
package com.huawei.oceanprotect.system.base.service;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigLanguage;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.enums.InstallationLanguageType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.DeviceManagerResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.api.SystemServiceApi;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.system.SystemContextService;
import openbackup.system.base.sdk.system.model.TimeZoneInfo;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.lang.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;

/**
 * 系统上下文服务接口实现
 *
 */
@Slf4j
@Service
public class SystemContextServiceImpl implements SystemContextService {
    @Autowired
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Autowired
    private LocalStorageService localStorageService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private ClusterBasicService clusterBasicService;

    @Autowired
    private SystemServiceApi systemServiceApi;

    @Override
    public String getSystemLanguage() {
        List<InitConfigInfo> initConfigInfos = initNetworkConfigMapper.queryInitConfig(
                InitConfigConstant.INSTALLATION_LANGUAGE_FLAG);
        if (initConfigInfos == null || initConfigInfos.size() == 0) {
            log.info("no Installation the Language");
            return InstallationLanguageType.NULL_LANGUAGE.getValue();
        }
        String initValue = initConfigInfos.get(0).getInitValue();
        ConfigLanguage installationLanguage = JSONObject.toBean(initValue, ConfigLanguage.class);

        return installationLanguage.getLanguage().getValue();
    }

    @Override
    public TimeZoneInfo getSystemTimeZone() {
        log.info("getSystemTimeZone start");
        DeviceManagerResponse<List<TimeZoneInfo>> response = null;
        if (deployTypeService.isXSeries() || deployTypeService.isPacific()) {
            String esn = clusterBasicService.getCurrentClusterEsn();
            if (!StringUtils.isEmpty(esn)) {
                response = systemServiceApi.queryBasicSystemTimezone(esn, UserUtils.getBusinessUsername());
            }
        } else {
            response = localStorageService.getLocalStorageDeviceManageService().getTimeZoneInfo();
        }
        if (response == null || VerifyUtil.isEmpty(response.getData())) {
            log.info("getSystemTimeZone response data is null");
            return new TimeZoneInfo();
        }
        return response.getData().get(0);
    }
}
