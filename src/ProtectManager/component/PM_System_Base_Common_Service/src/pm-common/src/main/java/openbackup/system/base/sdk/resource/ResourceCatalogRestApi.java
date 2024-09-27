/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.resource;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.resource.model.ResourceCatalogSchema;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

/**
 * get protected resource
 *
 * @author l00347293
 * @since 2021-01-04
 */
@FeignClient(name = "ResourceCatalogService", url = "${pm-resource-manager.url}/v1",
    configuration = CommonFeignConfiguration.class)
public interface ResourceCatalogRestApi {
    /**
     * 查询资源目录
     *
     * @return List<ResourceCatalogSchema>
     */
    @ExterAttack
    @GetMapping("/resource-catalogs")
    @ResponseBody
    List<ResourceCatalogSchema> queryResourceCatalog();

    /**
     * 隐藏资源目录
     *
     * @param catalogIds resource catalog uuid list
     */
    @PutMapping("/resource-catalogs/action/hidden")
    @ResponseBody
    void hiddenCatalog(@RequestBody List<String> catalogIds);

    /**
     * 展示资源目录
     *
     * @param catalogIds resource catalog uuid list
     */
    @PutMapping("/resource-catalogs/action/show")
    @ResponseBody
    void showCatalog(@RequestBody List<String> catalogIds);
}
