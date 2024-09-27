package openbackup.oceanbase.common.dto;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-08-01
 */
@Getter
@Setter
public class ResultMessage {
    private long errorCode;

    private List<String> parameters;
}
