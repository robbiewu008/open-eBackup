package openbackup.system.base.sdk.infrastructure.model.beans;

import lombok.Data;

/**
 * HA后置任务请求
 *
 * @author w00607005
 * @since 2023-05-22
 */
@Data
public class PostClusterHaRequest {
    /**
     * 操作类型，add、modify、remove
     */
    private String type;

    /**
     * 任务结果，success：成功，fail：失败
     */
    private String result;

    /**
     * 节点角色，PRIMARY：主节点，STANDBY：从节点
     */
    private String role;
}
