package openbackup.data.access.framework.livemount.controller.livemount.model;

import lombok.Data;

import org.hibernate.validator.constraints.Length;

import java.util.Map;

/**
 * Live Mount Param
 *
 * @author l00272247
 * @since 2020-09-17
 */
@Data
public class LiveMountParam {
    @Length(max = 64)
    private String policyId;

    private Map<String, Object> parameters;
}
