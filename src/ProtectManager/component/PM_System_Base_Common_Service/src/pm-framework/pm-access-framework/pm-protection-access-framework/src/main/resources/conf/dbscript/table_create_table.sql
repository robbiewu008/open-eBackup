--
-- PostgreSQL database dump
--

--  SET statement_timeout = 0;
--  SET client_encoding = 'SQL_ASCII';
--  SET standard_conforming_strings = on;
--  SET check_function_bodies = false;
--  SET client_min_messages = notice;

--
-- Name: SEARCHPATH; Type: SEARCHPATH; Schema: -; Owner: 
--

--  SET search_path = '';


--
-- Name: PLPGSQL; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS PLPGSQL WITH SCHEMA PG_CATALOG;


--
-- Name: EXTENSION PLPGSQL; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION PLPGSQL IS 'PL/pgSQL procedural language';


--
-- Name: DBLINK; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS DBLINK WITH SCHEMA PG_CATALOG;


--
-- Name: EXTENSION DBLINK; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION DBLINK IS 'connect to other PostgreSQL databases from within a database';


--  SET search_path = PUBLIC, PG_CATALOG;

--  SET default_tablespace = '';

--  SET default_with_oids = false;

--
-- Name: T_ALARM; Type: TABLE; Schema: PUBLIC; Owner: GAUSSDB; Tablespace: 
--

CREATE TABLE if not exists PUBLIC.LIVE_MOUNT_POLICY (
	POLICY_ID CHARACTER VARYING(128) NOT NULL,
	NAME CHARACTER VARYING(255) NOT NULL,

	COPY_DATA_SELECTION_POLICY CHARACTER VARYING(255) NOT NULL,

	SCHEDULE_POLICY CHARACTER VARYING(255) NOT NULL,
	SCHEDULE_INTERVAL INTEGER,
	SCHEDULE_INTERVAL_UNIT CHARACTER VARYING(255),
	SCHEDULE_START_TIME TIMESTAMP,

	RETENTION_POLICY CHARACTER VARYING(255) NOT NULL,
	RETENTION_VALUE INTEGER,
	RETENTION_UNIT CHARACTER VARYING(255),

	LATEST_COPY_FOR CHARACTER VARYING(255),
	AFTER_COPY_GENERATED CHARACTER VARYING(255),

	CREATED_TIME TIMESTAMP not null,
	UPDATED_TIME TIMESTAMP not null,
	USER_ID varchar(255),
	PRIMARY KEY (POLICY_ID)
);

CREATE TABLE if not exists PUBLIC.LIVE_MOUNT (
	ID CHARACTER VARYING(128) NOT NULL,
	RESOURCE_TYPE CHARACTER VARYING(64) NOT NULL,
	RESOURCE_SUB_TYPE CHARACTER VARYING(64) NOT NULL,
	RESOURCE_ID CHARACTER VARYING(128) NOT NULL,
	RESOURCE_NAME CHARACTER VARYING(128) NOT NULL,
	RESOURCE_PATH CHARACTER VARYING(256) NOT NULL,
	RESOURCE_IP CHARACTER VARYING(256),
	POLICY_ID CHARACTER VARYING(64),
	COPY_ID CHARACTER VARYING(64),
	TARGET_LOCATION CHARACTER VARYING(64) NOT NULL,
	TARGET_RESOURCE_ID CHARACTER VARYING(64) NOT NULL,
	TARGET_RESOURCE_NAME CHARACTER VARYING(128) NOT NULL,
	TARGET_RESOURCE_PATH CHARACTER VARYING(256) NOT NULL,
	TARGET_RESOURCE_IP CHARACTER VARYING(256),
	PARAMETERS CHARACTER VARYING ,
	ANONYMIZATION_STATUS INTEGER,
	STATUS CHARACTER VARYING(64) NOT NULL,
	ENABLE_STATUS CHARACTER VARYING(64) NOT NULL,
	CREATED_TIME TIMESTAMP NOT NULL,
	UPDATED_TIME TIMESTAMP NOT NULL,
	MOUNTED_COPY_ID CHARACTER VARYING(64),
	MOUNTED_COPY_DISPLAY_TIMESTAMP TIMESTAMP,
	MOUNTED_SOURCE_COPY_ID CHARACTER VARYING(64),
	SCHEDULE_ID CHARACTER VARYING(64),
	MOUNTED_RESOURCE_ID CHARACTER VARYING(64),
	USER_ID CHARACTER VARYING(255),
	PRIMARY KEY (ID)
);