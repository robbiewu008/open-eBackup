package openbackup.system.base.sdk.storage.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author p00511147
 * @since 2020-11-10
 */
@Data
public class OceanStorageLunInfoRes {
    @JsonProperty("ALLOCCAPACITY")
    private String allocCapacity;

    @JsonProperty("ALLOCTYPE")
    private String allocType;

    @JsonProperty("CAPABILITY")
    private String capability;

    @JsonProperty("CAPACITY")
    private String capacity;

    @JsonProperty("CAPACITYALARMLEVEL")
    private String capacityAlarmLevel;

    @JsonProperty("COMPRESSION")
    private String compression;

    @JsonProperty("COMPRESSIONSAVEDCAPACITY")
    private String compressionSavedCapacity;

    @JsonProperty("COMPRESSIONSAVEDRATIO")
    private String compressionSavedRation;

    @JsonProperty("DEDUPSAVEDCAPACITY")
    private String dedupSavedCapacity;

    @JsonProperty("DEDUPSAVEDRATIO")
    private String deduSavedRation;

    @JsonProperty("DESCRIPTION")
    private String description;

    @JsonProperty("DISGUISEREMOTEARRAYID")
    private String disguiseRemoteArrayId;

    @JsonProperty("DISGUISESTATUS")
    private String disguiseStatus;

    @JsonProperty("DRS_ENABLE")
    private String drsEnable;

    @JsonProperty("ENABLECOMPRESSION")
    private String enableCompression;

    @JsonProperty("ENABLEISCSITHINLUNTHRESHOLD")
    private String enableIscsiThinLunThreshold;

    @JsonProperty("ENABLESMARTDEDUP")
    private String enableSmartDedup;

    @JsonProperty("EXPOSEDTOINITIATOR")
    private String exposedToInitiator;

    @JsonProperty("EXTENDIFSWITCH")
    private String extenDIFSwith;

    @JsonProperty("HASRSSOBJECT")
    private String hasRSSObject;

    @JsonProperty("HEALTHSTATUS")
    private String healthStatus;

    @JsonProperty("HYPERCDPSCHEDULEDISABLE")
    private String hyperCdpScheduleDisable;

    @JsonProperty("ID")
    private String id;

    @JsonProperty("IOCLASSID")
    private String iOClassID;

    @JsonProperty("IOPRIORITY")
    private String iOPriority;

    @JsonProperty("ISADD2LUNGROUP")
    private String isAdd2LunGroup;

    @JsonProperty("ISCHECKZEROPAGE")
    private String isCheckZeroPage;

    @JsonProperty("ISCLONE")
    private String isClone;

    @JsonProperty("ISCSITHINLUNTHRESHOLD")
    private String iscsiThinLUNThreshold;

    @JsonProperty("MIRRORPOLICY")
    private String mirrorPolicy;

    @JsonProperty("MIRRORTYPE")
    private String mirrorType;

    @JsonProperty("NAME")
    private String name;

    @JsonProperty("NGUID")
    private String nguid;

    @JsonProperty("OWNINGCONTROLLER")
    private String owningController;

    @JsonProperty("PARENTID")
    private String parentID;

    @JsonProperty("PARENTNAME")
    private String parentName;

    @JsonProperty("PREFETCHPOLICY")
    private String prefetchPolicy;

    @JsonProperty("PREFETCHVALUE")
    private String prefetchValue;

    @JsonProperty("REMOTELUNID")
    private String remoteLunId;

    @JsonProperty("REPLICATION_CAPACITY")
    private String replicationCapacity;

    @JsonProperty("RUNNINGSTATUS")
    private String runningStatus;

    @JsonProperty("RUNNINGWRITEPOLICY")
    private String runningWritePolicy;

    @JsonProperty("SC_HITRAGE")
    private String scHitRage;

    @JsonProperty("SECTORSIZE")
    private String sectorSize;

    @JsonProperty("SMARTCACHEPARTITIONID")
    private String smartCachePartitionID;

    @JsonProperty("SNAPSHOTSCHEDULEID")
    private String snapshotScheduleId;

    @JsonProperty("SUBTYPE")
    private String subType;

    @JsonProperty("THINCAPACITYUSAGE")
    private String thinCapacityUsage;

    @JsonProperty("TOTALSAVEDCAPACITY")
    private String totalSavedCapacity;

    @JsonProperty("TOTALSAVEDRATIO")
    private String totalSavedRatio;

    @JsonProperty("TYPE")
    private String type;

    @JsonProperty("USAGETYPE")
    private String usageType;

    @JsonProperty("WORKINGCONTROLLER")
    private String workingController;

    @JsonProperty("WORKLOADTYPEID")
    private String workloadTypeId;

    @JsonProperty("WORKLOADTYPENAME")
    private String workloadTypeName;

    @JsonProperty("WRITEPOLICY")
    private String writePolicy;

    @JsonProperty("WWN")
    private String wwn;

    @JsonProperty("functionType")
    private String functionType;

    @JsonProperty("grainSize")
    private String grainSize;

    @JsonProperty("hyperCdpScheduleId")
    private String hyperCdpScheduleId;

    @JsonProperty("isShowDedupAndCompression")
    private String isShowDedupAndCompression;

    @JsonProperty("lunCgId")
    private String lunCgId;

    @JsonProperty("mapped")
    private String mapped;

    @JsonProperty("remoteLunWwn")
    private String remoteLunWwn;

    @JsonProperty("takeOverLunWwn")
    private String takeOverLunWwn;
}
