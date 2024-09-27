package openbackup.system.base.sdk.devicemanager.entity;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * pacific access zone的list
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-03
 */
@Getter
@Setter
public class ZoneResponseDto {
    // access zone的list
    private List<ZoneDto> data;
}
