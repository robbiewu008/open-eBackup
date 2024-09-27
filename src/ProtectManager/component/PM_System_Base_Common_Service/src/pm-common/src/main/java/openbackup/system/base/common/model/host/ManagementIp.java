package openbackup.system.base.common.model.host;

import lombok.Data;

import java.util.List;

/**
 * Agent当前控制器IP列表
 *
 * @author swx1010572
 * @since 2022-02-18
 */
@Data
public class ManagementIp {
    /**
     * PM-ip
     */
    private List<String> managerServerList;
}
