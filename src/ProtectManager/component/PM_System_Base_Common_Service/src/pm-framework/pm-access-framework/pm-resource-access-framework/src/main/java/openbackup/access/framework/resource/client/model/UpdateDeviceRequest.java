/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.client.model;

import lombok.AllArgsConstructor;
import lombok.Builder;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 更新设备请求体
 *
 * @author s30031954
 * @since 2022-12-29
 */
@Data
@Builder
@NoArgsConstructor
@AllArgsConstructor
public class UpdateDeviceRequest {
    /**
     * 设备ID
     */
    private String id;

    /**
     * 设备名称
     */
    private String name;

    /**
     * 设备状态
     */
    private String status;

    /**
     * 设备类型
     */
    private String type;

    /**
     * 设备地址
     */
    private String ip;

    /**
     *  设备端口
     */
    private String port;

    /**
     * 用户名称
     */
    private String userName;

    /**
     * 用户密码
     */
    private String userPwd;

    /**
     * 是否校验证书
     */
    private String enableCert;

    /**
     * 证书
     */
    private String certification;

    /**
     * 吊销列表
     */
    private String revocationList;

    /**
     * 所属用户
     */
    private String deviceBelongUserId;

    /**
     * 租户信息
     */
    private List<Vstore> vstores;
}
