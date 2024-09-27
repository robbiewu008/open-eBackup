package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

/**
 * FLRDestInfo
 *
 * @author p00511147
 * @since 2020-12-29
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class VmFlrDestInfo {
    private String vmIp;

    private String username;

    private String password;

    private String targetPath;
}
