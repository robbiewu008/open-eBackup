package openbackup.system.base.sdk.infrastructure.model.beans;

import lombok.Getter;
import lombok.Setter;

/**
 * 节点控制器信息
 *
 * @author g30003063
 * @since 2021-07-27
 */
@Getter
@Setter
public class NodeControllerInfo {
    /**
     * 控制器名称，例如：0A，0B
     */
    private String control;
}
