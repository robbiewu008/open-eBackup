package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * om-存储控制器节点信息
 *
 * @author p30001902
 * @since 2021-01-26
 */
@Data
public class NodeDetail {
    // 存储控制器管理ip地址，OM调用k8s接口获取
    private String address;

    private List<String> componentList;

    @JsonProperty(value = "hostname")
    private String hostName;

    private String nodeName;

    private String nodeStatus;
}
