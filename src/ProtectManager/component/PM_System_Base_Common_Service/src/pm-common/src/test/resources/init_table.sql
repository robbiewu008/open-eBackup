CREATE TABLE if not exists PUBLIC.T_EXTERNAL_ACCOUNT_INFO
(
    UUID VARCHAR(255) NOT NULL PRIMARY KEY,
    USER_NAME varchar(255) NOT NULL,
    USER_PWD varchar(1024),
    SOURCE_TYPE VARCHAR(64)
);