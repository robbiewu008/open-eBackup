package openbackup.system.base.sdk.dee.model;

import lombok.Data;

import java.util.List;

import javax.validation.constraints.Size;

/**
 * 功能描述
 *
 * @author c30047317
 * @since 2023-08-12
 */
@Data
public class ModifyEsClusterReq {
    // clusterInfo list
    @Size(min = 1, max = 64)
    List<EsNodeInfo> clusterInfo;

    // value: ADD;REMOVE
    @Size(min = 1, max = 10)
    private String operationType;
}