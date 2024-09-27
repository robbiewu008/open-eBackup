package openbackup.system.base.sdk.dee.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;

import java.util.List;

/**
 * 浏览恢复文件和目录返回体
 *
 * @author jwx701567
 * @since 2021-12-17
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class RestoreFilesResponse {
    private List<FineGrainedRestore> items;

    private Integer total;
}
