/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.auth.model.dao;

import static com.baomidou.mybatisplus.annotation.IdType.INPUT;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * 预置账号密码Po类
 *
 * @author y30021475
 * @since 2023-08-07
 */
@Getter
@Setter
@TableName(value = PresetAccountPo.TABLE_NAME)
public class PresetAccountPo {
    /**
     * table name
     */
    public static final String TABLE_NAME = "t_external_account_info";

    @TableId(type = INPUT)
    private String uuid;

    private String userName;

    private String userPwd;

    private String sourceType;
}
