package openbackup.tdsql.resources.access.dto.cluster;

import lombok.EqualsAndHashCode;
import lombok.Getter;
import lombok.Setter;

/**
 * OSS节点
 *
 * @author z30047175
 * @since 2023-05-23
 */
@Getter
@Setter
@EqualsAndHashCode(callSuper = true)
public class OssNode extends BaseNode {
    /**
     * 业务IP port端口
     */
    private String port;
}
