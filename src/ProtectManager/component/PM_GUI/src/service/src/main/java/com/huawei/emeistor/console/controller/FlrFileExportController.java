/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller;

import com.huawei.emeistor.console.contant.ConfigConstant;
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
class FlrFileExportController extends AdvBaseController {
    private static final String FLR_EXPORT = "/v1/flr/download";

    @Value("${api.gateway.endpoint}")
    private String flrFileExportApi;

    @Autowired
    private HttpServletRequest request;

    @Autowired
    private DownloadUtil downloadUtil;

    /**
     * FLR的下载
     */
    @GetMapping("/v1/flr/download")
    public void exportLogs() {
        String requestUrl =
            NormalizerUtil.normalizeForString(flrFileExportApi + FLR_EXPORT + "?" + request.getQueryString());
        downloadUtil.download(requestUrl);
    }
}
