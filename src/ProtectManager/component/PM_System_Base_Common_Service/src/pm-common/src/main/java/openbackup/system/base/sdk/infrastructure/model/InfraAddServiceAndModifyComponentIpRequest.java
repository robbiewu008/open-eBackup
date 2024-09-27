package openbackup.system.base.sdk.infrastructure.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 修改gaussDB/ES IP 请求体
 *
 * @author x30046484
 * @since 2023-05-19
 */

@Getter
@Setter
public class InfraAddServiceAndModifyComponentIpRequest {
    private String action;

    private List<String> ipList;

    @JsonProperty("needRestart")
    private boolean shouldRestart;
}
