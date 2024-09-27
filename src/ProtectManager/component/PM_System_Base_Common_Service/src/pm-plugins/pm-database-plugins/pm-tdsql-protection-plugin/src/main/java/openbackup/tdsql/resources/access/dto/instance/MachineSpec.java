/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */
/* $$$!!Warning: Huawei key information asset. No spread without permission.$$$ */
/* CODEMARK:rLr9evwLcHFK0ocH1vI5dv79JXrhoRrQ3pUSCsHdOM/puzvFwKILKNePe4bG/B98pz4V5doHi56R
0f3JfrYASX9dpcZzoFk8wkkpq5MAD4QH7vM9CsfnwL0y/c+rHjXKrQ8MVGc+XqcBtj8Yza8MaIQ4
/ST8U6y6HofoEwJrgIVvbNzNh9jwcLBXeEScWpiwYxW0zNwhHY1iBM67xOpgPAedtp5m1cUTn1w5
84t0/Td9XZ+UjLQlsmFxqQWLgEFi# */
/* $$$!!Warning: Deleting or modifying the preceding information is prohibited.$$$ */
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.dto.instance;

import lombok.Data;

import java.util.List;

/**
 * 分布式实例机型信息
 *
 * @author z00445440
 * @since 2023-11-27
 */
@Data
public class MachineSpec {
    List<Machine> machineList;
}
