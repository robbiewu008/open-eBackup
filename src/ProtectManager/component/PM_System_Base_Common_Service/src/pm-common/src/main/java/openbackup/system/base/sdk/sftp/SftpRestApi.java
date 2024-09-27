/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.sftp;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.infrastructure.model.beans.SftpUserInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.SftpUsernameInfo;
import openbackup.system.base.sdk.sftp.model.SftpResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.DeleteMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * sftp feign client
 *
 * @author dWX1009286
 * @since 2021-06-10
 */
@FeignClient(name = "InfrastructureSftpRestApi", url = "${service.url.sftp}/v1/infra/internal/sftp",
    configuration = CommonFeignConfiguration.class)
public interface SftpRestApi {
    /**
     * 调用SFTP容器接口创建SFTP用户
     *
     * @param sftpUserInfo SFTP用户信息
     * @return SftpResponse 结果：success:true or false
     */
    @ExterAttack
    @PostMapping("/add_user")
    @ResponseBody
    SftpResponse createSftpUser(@RequestBody SftpUserInfo sftpUserInfo);

    /**
     * 调用SFTP容器接口删除SFTP用户
     *
     * @param sftpUsernameInfo 用户名
     * @return SftpResponse 结果：success:true or false
     */
    @ExterAttack
    @DeleteMapping("/delete_user")
    @ResponseBody
    SftpResponse deleteSftpUser(@RequestBody SftpUsernameInfo sftpUsernameInfo);

    /**
     * 调用SFTP容器接口修改SFTP用户密码
     *
     * @param sftpUserInfo SFTP用户信息
     * @return SftpResponse 结果：success:true or false
     */
    @ExterAttack
    @PostMapping("/password")
    @ResponseBody
    SftpResponse modifySftpPassword(@RequestBody SftpUserInfo sftpUserInfo);
}
