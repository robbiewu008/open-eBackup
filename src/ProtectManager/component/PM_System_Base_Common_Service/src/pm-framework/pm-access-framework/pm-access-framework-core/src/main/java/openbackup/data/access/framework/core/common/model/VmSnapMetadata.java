package openbackup.data.access.framework.core.common.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 副本信息实体类
 *
 * @author y30037959
 * @since 2023-06-07
 */
@Getter
@Setter
@AllArgsConstructor
public class VmSnapMetadata {
    /**
     * 需要挂载的设备信息
     */
    @JsonProperty("disk_info")
    private List<DiskInfo> diskInfos;
}
