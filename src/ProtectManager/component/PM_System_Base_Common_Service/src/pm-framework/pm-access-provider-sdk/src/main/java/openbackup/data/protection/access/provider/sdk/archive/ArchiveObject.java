package openbackup.data.protection.access.provider.sdk.archive;

/**
 * This object is used to store the data required for creating an archive copy.
 *
 * @author y00490893
 * @version [OceanStor 100P 8.1.0]
 * @since 2020-06-17
 */
public class ArchiveObject {
    private String requestId;

    private String copyId;

    private String objectType;

    private String policy;

    private String jobId;

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getCopyId() {
        return copyId;
    }

    public void setCopyId(String copyId) {
        this.copyId = copyId;
    }

    public String getObjectType() {
        return objectType;
    }

    public void setObjectType(String objectType) {
        this.objectType = objectType;
    }

    public String getPolicy() {
        return policy;
    }

    public void setPolicy(String policy) {
        this.policy = policy;
    }

    public String getJobId() {
        return jobId;
    }

    public void setJobId(String jobId) {
        this.jobId = jobId;
    }
}
