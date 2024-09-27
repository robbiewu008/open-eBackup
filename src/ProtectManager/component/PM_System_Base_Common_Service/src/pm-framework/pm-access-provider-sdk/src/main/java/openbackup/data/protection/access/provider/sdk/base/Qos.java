package openbackup.data.protection.access.provider.sdk.base;

/**
 * Qos实体模型
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-01
 */
public class Qos {
    // 限制速率,单位M
    private int bandwidth;

    public int getBandwidth() {
        return bandwidth;
    }

    public void setBandwidth(int bandwidth) {
        this.bandwidth = bandwidth;
    }
}
