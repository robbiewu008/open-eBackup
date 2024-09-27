package openbackup.data.protection.access.provider.sdk.livemount;

import lombok.Getter;
import lombok.Setter;

/**
 * 即时挂载脚本
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/03/02
 */
@Getter
@Setter
public class LiveMountScript {
    private String preScript;

    private String postScript;

    private String failPostScript;
}