/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import lombok.Data;

/**
 * 数据库信息
 *
 * @author y00413474
 * @since 2020-07-30
 */
@Data
public class Database {
    private String instName;

    private String dbName;

    private String version;

    private Integer state;

    private Integer dbConfType;

    private Integer isAsmInst;

    private Integer authType;

    private Integer dbRole;

    private String oracleHome;

    private Integer isCluster;
}
