package openbackup.data.protection.access.provider.sdk.base.v2;

import lombok.Data;

import java.util.List;

/**
 * RestoreAppEnvironment
 *
 * @description: 恢复任务中的资源对应的环境信息
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
@Data
public class TaskEnvironment extends TaskCommonResource {
    /**
     * 访问地址，ip或者域名
     */
    private String endpoint;

    /**
     * 端口
     */
    private Integer port;

    /**
     * 集群环境对应的节点信息，环境为非集群环境时，该值为空
     */
    private List<TaskEnvironment> nodes;

    /**
     * 环境的连接状态，框架校验使用（序列化时忽略）
     */
    private String linkStatus;
}
