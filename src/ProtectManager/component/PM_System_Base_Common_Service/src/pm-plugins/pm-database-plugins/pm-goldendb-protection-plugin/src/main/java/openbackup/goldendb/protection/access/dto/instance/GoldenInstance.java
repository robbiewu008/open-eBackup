package openbackup.goldendb.protection.access.dto.instance;

import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 功能描述 GoldenDB实例
 *
 * @author s30036254
 * @since 2023-02-14
 */
@NoArgsConstructor
@Data
public class GoldenInstance {
    /**
     * id
     */
    private String id;

    /**
     * name
     */
    private String name;

    /**
     * group 分片
     */
    private List<Group> group;

    /**
     * gtm 全局事务节点
     */
    private List<Gtm> gtm;
}
