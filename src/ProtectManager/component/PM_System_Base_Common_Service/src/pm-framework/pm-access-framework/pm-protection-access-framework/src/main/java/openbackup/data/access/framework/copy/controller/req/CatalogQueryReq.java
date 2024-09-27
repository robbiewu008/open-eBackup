package openbackup.data.access.framework.copy.controller.req;

import lombok.Getter;
import lombok.Setter;

import javax.validation.constraints.NotBlank;

/**
 * CatalogQueryReq
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-29
 */
@Getter
@Setter
public class CatalogQueryReq {
    @NotBlank
    String parentPath;

    @NotBlank
    String name;

    String conditions;

    private int pageSize = 200;

    private int pageNum = 0;
}
