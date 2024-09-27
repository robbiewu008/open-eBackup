/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.instance;

import lombok.Data;

/**
 * 分布式实例机型信息
 *
 * @author z00445440
 * @since 2023-11-27
 */
@Data
public class Machine {
    String machine;
    int memory;
    int cpu;
    int dataDisk;
    int logDisk;
}
