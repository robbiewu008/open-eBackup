package openbackup.access.framework.resource.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 功能描述
 *
 * @author z00842230
 * @since 2024-04-09
 */
@AllArgsConstructor
@NoArgsConstructor
@Getter
@Setter
public class AgentRegisterResponse {
    /**
     * agent 注册成功uuid
     */
    @JsonProperty("uuid")
    private String uuid;
}
