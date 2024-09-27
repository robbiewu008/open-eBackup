/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.repository.api;

import openbackup.system.base.common.constants.LocalStorageInfoRes;
import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * 本地存储内部接口
 *
 * @author y30037959
 * @since 2022-12-08
 */
@FeignClient(name = "LocalStorageApi", url = "${pm-system-base.url}/v1/internal/local-storage",
    configuration = CommonFeignConfiguration.class)
public interface LocalStorageApi {
    /**
     * 获取本地存储（内部接口）
     *
     * @return LocalStorageInfoRes
     */
    @ExterAttack
    @GetMapping("/info")
    @ResponseBody
    LocalStorageInfoRes getStorageInfo();

    /**
     * 获取集群所有本地存储基本信息
     *
     * @return StorageInfoList
     */
    @ExterAttack
    @GetMapping("/info/list")
    @ResponseBody
    List<LocalStorageInfoRes> getStorageInfoList();
}
