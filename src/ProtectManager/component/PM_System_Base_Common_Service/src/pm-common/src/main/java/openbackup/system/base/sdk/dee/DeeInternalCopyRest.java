/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.dee;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.dee.model.CopyCatalogsRequest;
import openbackup.system.base.sdk.dee.model.RestoreFilesResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;

/**
 * dee 副本相关的内部接口
 *
 * @author jwx701567
 * @since 2021-12-25
 */
@FeignClient(name = "deeInternalCopyRest", url = "${protectengine-e-dee.url}/v1/internal",
        configuration = CommonFeignConfiguration.class)
public interface DeeInternalCopyRest {
    /**
     * 浏览副本文件和目录
     *
     * @param copyCatalogsRequest 副本文件和目录请求体
     * @return 副本文件和目录信息
     */
    @ExterAttack
    @PostMapping("/indexes/folder/action/list")
    RestoreFilesResponse listCopyCatalogs(@RequestBody CopyCatalogsRequest copyCatalogsRequest);
}
