package openbackup.system.base.sdk.infrastructure.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * 基础设施回应（带错误）
 *
 * @param <T> 可变参数
 * @author w00493811
 * @since 2021-01-25
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class InfraResponseWithError<T> {
    /**
     * 回应数据
     */
    @JsonProperty("data")
    private T data;

    /**
     * 回应错误码
     */
    @JsonProperty("error")
    private InfraResponseErrorInfo error;
}
