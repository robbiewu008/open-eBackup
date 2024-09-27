package openbackup.system.base.sdk.auth.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

/**
 * Dme Region对象属性
 *
 * @author z30062305
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-29
 */
@Setter
@Getter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class DmeProjectRegion {
    /**
     * Region Id
     */
    private String id;

    /**
     * Region名称
     */
    private String name;
}
