package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * QueryRegionAndProjectResponse
 *
 * @author y30021475
 * @since 2023-09-13
 */
@Getter
@Setter
public class QueryRegionAndProjectResponse {
    private int total;

    private List<HcsProject> projects;
}
