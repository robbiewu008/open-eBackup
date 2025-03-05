CREATE TABLE if not exists PUBLIC."RESOURCES"
(
    UUID              VARCHAR(64) NOT NULL,
    NAME              VARCHAR(256),
    TYPE              VARCHAR(64),
    USER_ID           VARCHAR(255),
    AUTHORIZED_USER   VARCHAR(255),
    SUB_TYPE          VARCHAR(64),
    SOURCE_TYPE       VARCHAR(16),
    PATH              VARCHAR(1024),
    PARENT_NAME       VARCHAR(256),
    PARENT_UUID       VARCHAR(64),
    ROOT_UUID         VARCHAR(64),
    DISCRIMINATOR     VARCHAR(64),
    CREATED_TIME      TIMESTAMP WITHOUT TIME ZONE NOT NULL,
    PROTECTION_STATUS INTEGER DEFAULT 0,
    VERSION           VARCHAR(64),
    AUTH              TEXT,
    CONSTRAINT "RESOURCES_pkey" PRIMARY KEY (UUID)
);

CREATE TABLE public.virtual_resource (
                                         uuid VARCHAR(64) NOT NULL,
                                         vm_ip VARCHAR(4096),
                                         env_ip VARCHAR(256),
                                         link_status INT,
                                         capacity INT,
                                         free_space VARCHAR(32),
                                         uncommitted VARCHAR(32),
                                         mo_id VARCHAR(32),
                                         children VARCHAR(32),
                                         is_template BOOLEAN,
                                         alias_type VARCHAR(64),
                                         alias_value VARCHAR(64),
                                         os_type VARCHAR(64),
                                         tags TEXT,
                                         instance_id VARCHAR(64),
                                         firmware VARCHAR(64),
                                         PRIMARY KEY (uuid),
                                         FOREIGN KEY (uuid) REFERENCES resources(uuid) ON DELETE CASCADE
);

CREATE TABLE public.t_resource_group_member (
                                                uuid VARCHAR(64) NOT NULL,
                                                resource_group_id VARCHAR(64) NOT NULL,
                                                source_id VARCHAR(64),
                                                source_sub_type VARCHAR(128),
                                                PRIMARY KEY (uuid)
);

CREATE TABLE IF NOT EXISTS PUBLIC."RES_EXTEND_INFO"
(
    UUID        VARCHAR(64)  NOT NULL,
    RESOURCE_ID VARCHAR(64)  NOT NULL,
    KEY         VARCHAR(128) NOT NULL,
    VALUE       TEXT,
    CONSTRAINT "RES_EXTEND_INFO_pkey" PRIMARY KEY (RESOURCE_ID, KEY),
    FOREIGN KEY (RESOURCE_ID) REFERENCES PUBLIC."RESOURCES" (UUID) ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS PUBLIC."ENVIRONMENTS"
(
    UUID          VARCHAR(64) NOT NULL,
    ENDPOINT      VARCHAR(128),
    PORT          VARCHAR(16),
    USER_NAME     VARCHAR(256),
    PASSWORD      VARCHAR(2048),
    LINK_STATUS   VARCHAR(32),
    LOCATION      VARCHAR(128),
    OS_TYPE       VARCHAR(32),
    OS_NAME       VARCHAR(128),
    SCAN_INTERVAL INT DEFAULT 3600,
    IS_CLUSTER    BOOLEAN,
    CERT_NAME     VARCHAR(128),
    TIME_ZONE     VARCHAR(64) DEFAULT NULL,
    CONSTRAINT "ENVIRONMENTS_pkey" PRIMARY KEY (UUID),
    FOREIGN KEY (UUID) REFERENCES PUBLIC."RESOURCES" (UUID) ON DELETE CASCADE
);

CREATE TABLE if not exists PUBLIC."PROTECTED_OBJECT"
(
    UUID VARCHAR(64) NOT NULL,
    SLA_ID VARCHAR(64),
    SLA_NAME VARCHAR(64),
    NAME VARCHAR(128),
    ENV_ID VARCHAR(64),
    ENV_TYPE VARCHAR(16),
    RESOURCE_ID VARCHAR(64) NOT NULL,
    TYPE VARCHAR(64) NOT NULL,
    SUB_TYPE VARCHAR(64),
    SLA_COMPLIANCE BOOLEAN,
    PATH TEXT,
    EXT_PARAMETERS JSON,
    STATUS INTEGER,
    LATEST_TIME TIMESTAMP WITHOUT TIME ZONE,
    EARLIEST_TIME TIMESTAMP WITHOUT TIME ZONE,
    CHAIN_ID VARCHAR(128) NOT NULL,
    CONSISTENT_STATUS VARCHAR(64),
    CONSISTENT_RESULTS VARCHAR(64),
    RESOURCE_GROUP_ID VARCHAR(64),
    CONSTRAINT "PROTECTED_OBJECT_pkey" PRIMARY KEY (UUID)
);

CREATE TABLE if not exists PUBLIC.T_HOST_AGENT_INFO (
    UUID CHARACTER VARYING(64) NOT NULL,
    CPU_RATE DECIMAL,
    MEM_RATE DECIMAL,
    LAST_UPDATE_TIME BIGINT,
    IS_SHARED BOOLEAN DEFAULT FALSE
);