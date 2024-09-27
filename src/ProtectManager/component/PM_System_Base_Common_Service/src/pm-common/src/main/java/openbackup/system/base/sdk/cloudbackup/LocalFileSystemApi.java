/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.cloudbackup;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;
import openbackup.system.base.common.constants.HyperMetroPair;
import openbackup.system.base.common.constants.LocalFileSystem;
import openbackup.system.base.common.constants.LocalRemoteReplicationPair;
import openbackup.system.base.common.rest.CommonFeignConfiguration;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;

/**
 * The CloudBackupApi
 *
 * @author g30003063
 * @since 2021-12-14
 */
@FeignClient(name = "localFileSystemApi", url = "${service.url.pm-repository}/v1/internal/local-storage",
    configuration = CommonFeignConfiguration.class)
public interface LocalFileSystemApi {
    /**
     * 通过ID查找本地文件系统
     *
     * @param id ID
     * @return 本地文件系统
     */
    @ExterAttack
    @GetMapping("/filesystem/{id}")
    LocalFileSystem getLocalFileSystem(@PathVariable(name = "id") String id);

    /**
     * 通过ID查询本地远程复制Pair
     *
     * @param id ID
     * @return 本地远程复制Pair
     */
    @ExterAttack
    @GetMapping("/replicationpair/{id}")
    LocalRemoteReplicationPair getLocalRemoteReplicationPair(@PathVariable(name = "id") String id);

    /**
     * 通过ID查询双活Pair
     *
     * @param id ID
     * @return 文件系统双活域
     */
    @ExterAttack
    @GetMapping("/hypermetropair/{id}")
    HyperMetroPair getHyperMetroPair(@PathVariable(name = "id") String id);
}
