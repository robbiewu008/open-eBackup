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
