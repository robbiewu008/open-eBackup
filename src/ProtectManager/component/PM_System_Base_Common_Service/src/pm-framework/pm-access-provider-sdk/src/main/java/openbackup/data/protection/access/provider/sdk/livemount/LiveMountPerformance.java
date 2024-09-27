package openbackup.data.protection.access.provider.sdk.livemount;

import com.fasterxml.jackson.annotation.JsonAlias;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Performance
 *
 * @author l00272247
 * @since 2020-09-21
 */
@Data
@JsonInclude(JsonInclude.Include.NON_DEFAULT)
public class LiveMountPerformance {
    @JsonProperty("BandwidthMin")
    @JsonAlias("min_bandwidth")
    private int bandwidthMin;

    @JsonProperty("BandwidthMax")
    @JsonAlias("max_bandwidth")
    private int bandwidthMax;

    @JsonProperty("BandwidthBurst")
    @JsonAlias("burst_bandwidth")
    private int bandwidthBurst;

    @JsonProperty("IOPSMin")
    @JsonAlias("min_iops")
    private int iopsMin;

    @JsonProperty("IOPSMax")
    @JsonAlias("max_iops")
    private int iopsMax;

    @JsonProperty("IOPSBurst")
    @JsonAlias("burst_iops")
    private int iopsBurst;

    @JsonProperty("BurstTime")
    @JsonAlias("burst_time")
    private int burstTime;

    @JsonProperty("Latency")
    @JsonAlias("latency")
    private int latency;

    private int fileSystemKeepTime;
}
