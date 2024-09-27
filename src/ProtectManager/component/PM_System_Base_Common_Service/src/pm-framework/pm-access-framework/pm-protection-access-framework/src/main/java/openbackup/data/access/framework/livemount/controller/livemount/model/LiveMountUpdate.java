package openbackup.data.access.framework.livemount.controller.livemount.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import javax.validation.constraints.NotNull;

/**
 * Live Mount Update
 *
 * @author l00272247
 * @since 2020-09-17
 */
@Data
public class LiveMountUpdate {
    @NotNull
    private LiveMountUpdateMode mode;

    @JsonProperty("copy_id")
    private String copyId;
}
