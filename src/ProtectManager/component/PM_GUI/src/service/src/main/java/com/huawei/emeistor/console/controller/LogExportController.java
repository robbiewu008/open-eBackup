/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
 * @author z00560567
 * @since 2021-01-12
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
