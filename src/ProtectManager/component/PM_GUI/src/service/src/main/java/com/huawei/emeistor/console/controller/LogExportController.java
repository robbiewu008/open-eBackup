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
package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
import com.huawei.emeistor.console.exterattack.ExterAttack;
import com.huawei.emeistor.console.util.DownloadUtil;
import com.huawei.emeistor.console.util.NormalizerUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import javax.servlet.http.HttpServletRequest;

/**
 * 功能描述
 *
 */
@RestController
@RequestMapping(ConfigConstant.CONSOLE)
class LogExportController extends AdvBaseController {
    private static final String LOG_EXPORT = "/v1/infra/logs/export";

    @Value("${api.gateway.endpoint}")
    private String logExportApi;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private DownloadUtil downloadUtil;

    /**
     * 日志下载
     */
    @ExterAttack
    @GetMapping("/v1/infra/logs/export")
    public void exportLogs() {
        String requestUrl =
            NormalizerUtil.normalizeForString(logExportApi + LOG_EXPORT + "?" + request.getQueryString());
        downloadUtil.download(requestUrl);
    }
}
