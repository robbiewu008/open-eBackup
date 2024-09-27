package openbackup.system.base.sdk.exportfile.model;

import lombok.Data;

/**
 * LogLevelInfoDto
 *
 * @author w00607005
 * @since 2023-07-24
 */
@Data
public class LogLevelDto {
    /**
     * esn
     */
    private String esn;

    /**
     * 节点名称
     */
    private String nodeName;

    /**
     * 节点ip
     */
    private String nodeIp;

    /**
     * 节点角色
     */
    private int role;

    /**
     * 节点状态
     */
    private int status;

    /**
     * 日志等级
     */
    private String logLevel;
}
