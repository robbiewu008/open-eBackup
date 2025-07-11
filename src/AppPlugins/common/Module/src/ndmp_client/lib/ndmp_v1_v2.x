/*							   -*- Mode: C -*- 
 * ndmp.x
 * 
 * Description	 : NDMP protocol rpcgen file.
 * 
 * Copyright (c) 1999 Network Appliance, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

%#ifdef RPC_HDR
%#if !defined(lint) && !defined(SABER)
%static char ndmp_h_rcsId[] = "@(#) $Id: //depot/tools/main/ndmptools/ndmpcopy-1.2/lib/ndmp_v1_v2.x#1 $";
%#endif
%#endif

#define VER 2
const NDMPVER = VER;
const NDMPPORT = 10000;

struct ndmp_u_quad
{
	u_long	high;
	u_long	low;
};

struct ndmp_pval
{
	string	name<>;
	string	value<>;
};

struct ndmp_scsi_device
{
	string	name<>;
};

struct ndmp_tape_device
{
	string	name<>;
};

enum ndmp_error
{
	NDMP_NO_ERR,					/* No error */
	NDMP_NOT_SUPPORTED_ERR,			/* Call is not supported */
	NDMP_DEVICE_BUSY_ERR,			/* The device is in use */
	NDMP_DEVICE_OPENED_ERR,			/* Another tape or scsi device  
									   is already open */
	NDMP_NOT_AUTHORIZED_ERR,		/* connection has not been authorized */
	NDMP_PERMISSION_ERR,			/* some sort of permission problem */
	NDMP_DEV_NOT_OPEN_ERR,			/* SCSI device is not open */
	NDMP_IO_ERR,					/* I/O error */
	NDMP_TIMEOUT_ERR,				/* command timed out */
	NDMP_ILLEGAL_ARGS_ERR,			/* illegal arguments in request */
	NDMP_NO_TAPE_LOADED_ERR,	/* Cannot open because there is 
									   no tape loaded */
	NDMP_WRITE_PROTECT_ERR,			/* tape cannot be open for write */
	NDMP_EOF_ERR,					/* Command encountered EOF */
	NDMP_EOM_ERR,					/* Command encountered EOM */
	NDMP_FILE_NOT_FOUND_ERR,		/* File not found during restore */
	NDMP_BAD_FILE_ERR,				/* The file descriptor is invalid */
	NDMP_NO_DEVICE_ERR,				/* The device is not at that target */
	NDMP_NO_BUS_ERR,				/* Invalid controller */
	NDMP_XDR_DECODE_ERR,			/* Can't decode the request argument */
	NDMP_ILLEGAL_STATE_ERR,			/* Call can't be performed at this state */
	NDMP_UNDEFINED_ERR,				/* Undefined Error */
	NDMP_XDR_ENCODE_ERR,			/* Can't encode the reply argument */
	NDMP_NO_MEM_ERR					/* no memory */

};

enum ndmp_header_message_type
{
	NDMP_MESSAGE_REQUEST,
	NDMP_MESSAGE_REPLY
};

enum ndmp_message
{
	NDMP_CONNECT_OPEN = 0x900,			/* CONNECT INTERFACE */
	NDMP_CONNECT_AUTH,
#if VER > 1
	NDMP_CONNECT_CLIENT_AUTH = NDMP_CONNECT_AUTH,
#endif
	NDMP_CONNECT_CLOSE,
#if VER > 1
	NDMP_CONNECT_SERVER_AUTH,
#endif


	NDMP_CONFIG_GET_HOST_INFO = 0x100,	/* CONFIG INTERFACE */
	NDMP_CONFIG_GET_BUTYPE_ATTR,
#if VER > 1
	NDMP_CONFIG_GET_MOVER_TYPE,
	NDMP_CONFIG_GET_AUTH_ATTR,
#endif

	NDMP_SCSI_OPEN = 0x200,				/* SCSI INTERFACE */
	NDMP_SCSI_CLOSE,
	NDMP_SCSI_GET_STATE,
	NDMP_SCSI_SET_TARGET,
	NDMP_SCSI_RESET_DEVICE,
	NDMP_SCSI_RESET_BUS,
	NDMP_SCSI_EXECUTE_CDB,

	NDMP_TAPE_OPEN = 0x300,				/* TAPE INTERFACE */
	NDMP_TAPE_CLOSE,
	NDMP_TAPE_GET_STATE,
	NDMP_TAPE_MTIO,
	NDMP_TAPE_WRITE,
	NDMP_TAPE_READ,
	NDMP_TAPE_SET_RECORD_SIZE,			/* V1 only */
	NDMP_TAPE_EXECUTE_CDB,

	NDMP_DATA_GET_STATE = 0x400,		/* DATA INTERFACE */
	NDMP_DATA_START_BACKUP,
	NDMP_DATA_START_RECOVER,
	NDMP_DATA_ABORT,
	NDMP_DATA_GET_ENV,
	NDMP_DATA_RESVD1,
	NDMP_DATA_RESVD2,
	NDMP_DATA_STOP,
	NDMP_DATA_CONTINUE,					/* V1 only */

	NDMP_NOTIFY_PAUSED = 0x500,			/* V1 only *//* NOTIFY INTERFACE */
	NDMP_NOTIFY_HALTED,
#if VER > 1
	NDMP_NOTIFY_DATA_HALTED = NDMP_NOTIFY_HALTED,
#endif
	NDMP_NOTIFY_CONNECTED,
#if VER > 1
	NDMP_NOTIFY_MOVER_HALTED,
	NDMP_NOTIFY_MOVER_PAUSED,
	NDMP_NOTIFY_DATA_READ,
#endif

	NDMP_LOG_LOG = 0x600,				/* LOGGING INTERFACE */
	NDMP_LOG_DEBUG,
	NDMP_LOG_FILE,
	NDMP_LOG_MESSAGE,

	NDMP_FH_ADD_UNIX_PATH = 0x700,		/* FILE HISTORY INTERFACE */
	NDMP_FH_ADD_UNIX_DIR,
	NDMP_FH_ADD_UNIX_NODE,

#if VER > 1
	NDMP_MOVER_GET_STATE = 0xa00,		/* MOVER INTERFACE */
	NDMP_MOVER_LISTEN,
	NDMP_MOVER_CONTINUE,
	NDMP_MOVER_ABORT,
	NDMP_MOVER_STOP,
	NDMP_MOVER_SET_WINDOW,
	NDMP_MOVER_READ,
	NDMP_MOVER_CLOSE,
	NDMP_MOVER_SET_RECORD_SIZE,
#endif

	NDMP_RESERVED = 0xff00				/* Reserved for prototyping */

};

struct ndmp_header
{
	u_long						sequence;		/* monotonically increasing number */
	u_long						time_stamp;		/* time stamp of message */
	ndmp_header_message_type	message_type;	/* what type of message */
	enum ndmp_message			message;		/* message number */
	u_long						reply_sequence;	/* reply is in response to */
	ndmp_error					error;			/* communications errors */
};

/**********************/
/*  CONNECT INTERFACE */
/**********************/
/* NDMP_CONNECT_OPEN */
struct ndmp_connect_open_request
{
	u_short	protocol_version;	/* the version of protocol supported */
};

struct ndmp_connect_open_reply
{
	ndmp_error	error;
};

/* NDMP_CONNECT_AUTH */
enum ndmp_auth_type
{
	NDMP_AUTH_NONE,		/* no password is required */
#if VER > 1
	NDMP_AUTH_TEXT,		/* the clear text password */
	NDMP_AUTH_MD5		/* md5 */
#else
	NDMP_AUTH_TEXT		/* the clear text password */
#endif
};

struct ndmp_auth_text
{
	string	user<>;
	string	password<>;
};

#if VER > 1
struct ndmp_auth_md5
{
	string	user<>;
	opaque	auth_digest[16];
};
#endif

union ndmp_auth_data switch (enum ndmp_auth_type auth_type)
{
	case NDMP_AUTH_NONE:
		void;
	case NDMP_AUTH_TEXT:
		struct ndmp_auth_text	auth_text;
#if VER > 1
	case NDMP_AUTH_MD5:
		struct ndmp_auth_md5	auth_md5;
#endif
};

struct ndmp_connect_auth_request
{
	ndmp_auth_data	auth_data;
};

struct ndmp_connect_auth_reply
{
	ndmp_error	error;
};

#if VER > 1
typedef ndmp_connect_auth_request	ndmp_connect_client_auth_request;
typedef	ndmp_connect_auth_reply		ndmp_connect_client_auth_reply;
#endif


/* NDMP_CONNECT_CLOSE */
/* no request arguments */
/* no reply arguments */


#if VER > 1
/* NDMP_CONNECT_SERVER_AUTH */
union ndmp_auth_attr switch (enum ndmp_auth_type auth_type)
{
	case NDMP_AUTH_NONE:
		void;
	case NDMP_AUTH_TEXT:
		void;
	case NDMP_AUTH_MD5:
	opaque	challenge[64];
};

struct ndmp_connect_server_auth_request
{
	ndmp_auth_attr	client_attr;
};

struct ndmp_connect_server_auth_reply
{
	ndmp_error	error;
	ndmp_auth_data	auth_result;
};


#endif
/********************/
/* CONFIG INTERFACE */
/********************/
/* NDMP_CONFIG_GET_HOST_INFO */
/* no request arguments */
struct ndmp_config_get_host_info_reply
{
	ndmp_error			error;
	string				hostname<>;	/* host name */
	string				os_type<>;	/* The operating system type (i.e.
									 * SOLARIS) */
	string				os_vers<>;	/* The version number of the OS (i.e.
									 * 2.5) */
	string				hostid<>;
	ndmp_auth_type		auth_type<>;
};

/* NDMP_CONFIG_GET_BUTYPE_ATTR */
const NDMP_NO_BACKUP_FILELIST	= 0x0001;
const NDMP_NO_BACKUP_FHINFO		= 0x0002;
const NDMP_NO_RECOVER_FILELIST	= 0x0004;
const NDMP_NO_RECOVER_FHINFO	= 0x0008;
const NDMP_NO_RECOVER_SSID		= 0x0010;
const NDMP_NO_RECOVER_INC_ONLY	= 0x0020;

struct ndmp_config_get_butype_attr_request
{
	string	name<>;		/* backup type name */
};

struct ndmp_config_get_butype_attr_reply
{
	ndmp_error	error;
	u_long		attrs;
};

#if VER > 1
/* NDMP_CONFIG_GET_MOVER_TYPE */
/* no request arguments */
enum ndmp_mover_addr_type
{
	NDMP_ADDR_LOCAL,
	NDMP_ADDR_TCP
};

struct ndmp_config_get_mover_type_reply
{
	ndmp_error				error;
	ndmp_mover_addr_type	methods<>;
};

/* NDMP_CONFIG_GET_AUTH_ATTR */
struct ndmp_config_get_auth_attr_request
{
	ndmp_auth_type	auth_type;
};

struct ndmp_config_get_auth_attr_reply
{
	ndmp_error	error;
	ndmp_auth_attr	server_attr;
};
#endif

/******************/
/* SCSI INTERFACE */
/******************/
/* NDMP_SCSI_OPEN */
struct ndmp_scsi_open_request
{
	ndmp_scsi_device	device;
};

struct ndmp_scsi_open_reply
{
	ndmp_error	error;
};

/* NDMP_SCSI_CLOSE */
/* no request arguments */
struct ndmp_scsi_close_reply
{
	ndmp_error	error;
};

/* NDMP_SCSI_GET_STATE */
/* no request arguments */
struct ndmp_scsi_get_state_reply
{
	ndmp_error	error;
	short		target_controller;
	short		target_id;
	short		target_lun;
};

/* NDMP_SCSI_SET_TARGET */
struct ndmp_scsi_set_target_request
{
	ndmp_scsi_device	device;
	u_short				target_controller;
	u_short				target_id;
	u_short				target_lun;
};

struct ndmp_scsi_set_target_reply
{
	ndmp_error	error;
};

/* NDMP_SCSI_RESET_DEVICE */
/* no request arguments */
struct ndmp_scsi_reset_device_reply
{
	ndmp_error	error;
};

/* NDMP_SCSI_RESET_BUS */
/* no request arguments */
struct ndmp_scsi_reset_bus_reply
{
	ndmp_error	error;
};

/* NDMP_SCSI_EXECUTE_CDB */
const NDMP_SCSI_DATA_IN		= 0x00000001;	/* Expect data from SCSI device */
const NDMP_SCSI_DATA_OUT	= 0x00000002;	/* Transfer data to SCSI device */

struct ndmp_execute_cdb_request
{
	u_long	flags;
	u_long	timeout;
	u_long	datain_len;		/* Set for expected datain */
	opaque	cdb<>;
	opaque	dataout<>;
};

struct ndmp_execute_cdb_reply
{
	ndmp_error	error;
	u_char		status;			/* SCSI status bytes */
	u_long		dataout_len;
	opaque		datain<>;		/* SCSI datain */
	opaque		ext_sense<>;	/* Extended sense data */
};

/******************/
/* TAPE INTERFACE */
/******************/
/* NDMP_TAPE_OPEN */
enum ndmp_tape_open_mode
{
	NDMP_TAPE_READ_MODE,
	NDMP_TAPE_WRITE_MODE
};

struct ndmp_tape_open_request
{
	ndmp_tape_device	device;
	ndmp_tape_open_mode	mode;
};

struct ndmp_tape_open_reply
{
	ndmp_error	error;
};

/* NDMP_TAPE_CLOSE */
/* no request arguments */
struct ndmp_tape_close_reply
{
	ndmp_error	error;
};

/*NDMP_TAPE_GET_STATE */
/* no request arguments */
const NDMP_TAPE_NOREWIND	= 0x0008;	/* non-rewind device */
const NDMP_TAPE_WR_PROT		= 0x0010;	/* write-protected */
const NDMP_TAPE_ERROR		= 0x0020;	/* media error */
const NDMP_TAPE_UNLOAD		= 0x0040;	/* tape will be unloaded when
										 * the device is closed */

struct ndmp_tape_get_state_reply
{
	ndmp_error	error;
	u_long		flags;
	u_long		file_num;
	u_long		record_num;
	u_long		record_size;
	u_long		soft_errors;
	u_long		block_size;
	u_long		blockno;
	ndmp_u_quad	total_space;
	ndmp_u_quad	space_remain;
};

#if VER > 1
struct ndmp_tape_get_state_reply_v2
{
	ndmp_error	error;
	u_long		flags;
	u_long		file_num;
	u_long		soft_errors;
	u_long		block_size;
	u_long		blockno;
	ndmp_u_quad	total_space;
	ndmp_u_quad	space_remain;
};
#endif

/* NDMP_TAPE_MTIO */
enum ndmp_tape_mtio_op
{
	NDMP_MTIO_FSF,
	NDMP_MTIO_BSF,
	NDMP_MTIO_FSR,
	NDMP_MTIO_BSR,
	NDMP_MTIO_REW,
	NDMP_MTIO_EOF,
	NDMP_MTIO_OFF
};

struct ndmp_tape_mtio_request
{
	ndmp_tape_mtio_op	tape_op;
	u_long				count;
};

struct ndmp_tape_mtio_reply
{
	ndmp_error	error;
	u_long		resid_count;
};

/* NDMP_TAPE_WRITE */
struct ndmp_tape_write_request
{
	opaque	data_out<>;
};

struct ndmp_tape_write_reply
{
	ndmp_error	error;
	u_long		count;
};

/* NDMP_TAPE_READ */
struct ndmp_tape_read_request
{
	u_long	count;
};

struct ndmp_tape_read_reply
{
	ndmp_error	error;
	opaque		data_in<>;
};

/* NDMP_TAPE_SET_RECORD_SIZE */
struct ndmp_tape_set_record_size_request
{
	u_long	len;
};

struct ndmp_tape_set_record_size_reply
{
	ndmp_error	error;
};

/* NDMP_TAPE_EXECUTE_CDB */
typedef ndmp_execute_cdb_request	ndmp_tape_execute_cdb_request;
typedef ndmp_execute_cdb_reply		ndmp_tape_execute_cdb_reply;


/********************************/
/* MOVER INTERFACE              */
/********************************/
#if VER > 1
/* MOVER_GET_STATE */
enum ndmp_mover_state
{
	NDMP_MOVER_STATE_IDLE,
	NDMP_MOVER_STATE_LISTEN,
	NDMP_MOVER_STATE_ACTIVE,
	NDMP_MOVER_STATE_PAUSED,
	NDMP_MOVER_STATE_HALTED
};

enum ndmp_mover_pause_reason
{
	NDMP_MOVER_PAUSE_NA,
	NDMP_MOVER_PAUSE_EOM,
	NDMP_MOVER_PAUSE_EOF,
	NDMP_MOVER_PAUSE_SEEK,
	NDMP_MOVER_PAUSE_MEDIA_ERROR
};

enum ndmp_mover_halt_reason
{
	NDMP_MOVER_HALT_NA,
	NDMP_MOVER_HALT_CONNECT_CLOSED,
	NDMP_MOVER_HALT_ABORTED,
	NDMP_MOVER_HALT_INTERNAL_ERROR,
	NDMP_MOVER_HALT_CONNECT_ERROR
};

/* no request arguments */
struct ndmp_mover_get_state_reply
{
	ndmp_error				error;
	ndmp_mover_state		state;
	ndmp_mover_pause_reason	pause_reason;
	ndmp_mover_halt_reason	halt_reason;
	u_long					record_size;
	u_long					record_num;
	ndmp_u_quad				data_written;
	ndmp_u_quad				seek_position;
	ndmp_u_quad				bytes_left_to_read;
	ndmp_u_quad				window_offset;
	ndmp_u_quad				window_length;
};

/* MOVER_LISTEN */
enum ndmp_mover_mode
{
	NDMP_MOVER_MODE_READ,	/* read from data connection; write to tape */
	NDMP_MOVER_MODE_WRITE,	/* write to data connection; read from tape */
	NDMP_MOVER_MODE_DATA	/* write to data connection; read from data connection */
};

struct ndmp_mover_tcp_addr
{
	u_long	ip_addr;
	u_short	port;
};
union ndmp_mover_addr switch (ndmp_mover_addr_type addr_type)
{
	case NDMP_ADDR_LOCAL:
		void;
	case NDMP_ADDR_TCP:
	  ndmp_mover_tcp_addr	addr;
};

struct ndmp_mover_listen_request
{
	ndmp_mover_mode			mode;
	ndmp_mover_addr_type	addr_type;
};

struct ndmp_mover_listen_reply
{
	ndmp_error		error;
	ndmp_addr_v4	mover;
};

/* MOVER_SET_RECORD_SIZE */
struct ndmp_mover_set_record_size_request
{
	u_long	len;
};

struct ndmp_mover_set_record_size_reply
{
	ndmp_error	error;
};

/* MOVER_SET_WINDOW */
struct ndmp_mover_set_window_request
{
	ndmp_u_quad	offset;
	ndmp_u_quad	length;
};

struct ndmp_mover_set_window_reply
{
	ndmp_error	error;
};

/* MOVER_CONTINUE */
/* no request arguments */
struct ndmp_mover_continue_reply
{
	ndmp_error	error;
};


/* MOVER_ABORT */
/* no request arguments */
struct ndmp_mover_abort_reply
{
	ndmp_error	error;
};

/* MOVER_STOP */
/* no request arguments */
struct ndmp_mover_stop_reply
{
	ndmp_error	error;
};

/* MOVER_READ */
struct ndmp_mover_read_request
{
	ndmp_u_quad	offset;
	ndmp_u_quad	length;
};

struct ndmp_mover_read_reply
{
	ndmp_error	error;
};

/* MOVER_CLOSE */
/* no request arguments */
struct ndmp_mover_close_reply
{
	ndmp_error	error;
};

#endif
/********************************/
/* DATA INTERFACE			    */
/********************************/
/* NDMP_DATA_GET_STATE */
/* no request arguments */
enum ndmp_data_operation
{
	NDMP_DATA_OP_NOACTION,
	NDMP_DATA_OP_BACKUP,
	NDMP_DATA_OP_RESTORE
};

enum ndmp_data_state
{
	NDMP_DATA_STATE_IDLE,
	NDMP_DATA_STATE_ACTIVE,
	NDMP_DATA_STATE_PAUSED,
	NDMP_DATA_STATE_HALTED
};

#if VER > 1
enum ndmp_data_state_v2
{
	NDMP_DATA_STATE_IDLE_v2,
	NDMP_DATA_STATE_ACTIVE_v2,
	NDMP_DATA_STATE_HALTED_v2
};
#endif
enum ndmp_data_halt_reason
{
	NDMP_HALT_NA,
	NDMP_HALT_SUCCESSFUL,
	NDMP_HALT_ABORTED,
	NDMP_HALT_MEDIA_ERROR,
	NDMP_HALT_INTERNAL_ERROR,
	NDMP_HALT_NO_SPLIT
};

#if VER > 1
enum ndmp_data_halt_reason_v2
{
	NDMP_DATA_HALT_NA,
	NDMP_DATA_HALT_SUCCESSFUL,
	NDMP_DATA_HALT_ABORTED,
	NDMP_DATA_HALT_INTERNAL_ERROR,
	NDMP_DATA_HALT_CONNECT_ERROR
};
#endif

enum ndmp_data_pause_reason
{
	NDMP_PAUSE_NA,
	NDMP_PAUSE_EOM,
	NDMP_PAUSE_EOF,
	NDMP_PAUSE_RSVD
};

struct ndmp_data_get_state_reply
{
	ndmp_error				error;
	ndmp_data_operation		operation;
	ndmp_data_state			state;
	ndmp_data_halt_reason	halt_reason;
	ndmp_data_pause_reason	pause_reason;
	ndmp_u_quad				reserved1;
	ndmp_u_quad				bytes_processed;
	ndmp_u_quad				est_bytes_remain;
	u_long					est_time_remain;
	ndmp_u_quad				reserved2;
	ndmp_u_quad				reserved3;
};

#if VER > 1
struct ndmp_data_get_state_reply_v2
{
	ndmp_error					error;
	ndmp_data_operation			operation;
	ndmp_data_state_v2			state;
	ndmp_data_halt_reason_v2	halt_reason;
	ndmp_u_quad					bytes_processed;
	ndmp_u_quad					est_bytes_remain;
	u_long						est_time_remain;
	ndmp_mover_addr				mover;
	ndmp_u_quad					read_offset;
	ndmp_u_quad					read_length;
};
#endif

/* NDMP_DATA_START_BACKUP */
struct ndmp_data_start_backup_request
{
	string		bu_type<>;	/* backup method to use */
	ndmp_pval	env<>;		/* Parameters that may modify backup */
};

#if VER > 1
struct ndmp_data_start_backup_request
{
	string			bu_type<>;	/* backup method to use */
	ndmp_pval		env<>;		/* Parameters that may modify backup */
};
#endif

struct ndmp_data_start_backup_reply
{
	ndmp_error	error;
};

/* NDMP_DATA_START_RECOVER */
struct ndmp_name
{
	string		name<>;
	string		dest<>;
	u_short		ssid;
	ndmp_u_quad	fh_info;
};

struct ndmp_data_start_recover_request
{
	ndmp_pval	env<>;
	ndmp_name	nlist<>;
	string		bu_type<>;
};

/* NDMP_DATA_START_RECOVER */
struct ndmp_name_v3
{
	string		original_path<>;
	string		destination_dir<>;
	string		new_name<>;	/* Direct access restore only */
	string		other_name<>;	/* Direct access restore only */
	ndmp_u_quad	node;		/* Direct access restore only */
	ndmp_u_quad	fh_info;	/* Direct access restore only */
};

#if VER > 1
struct ndmp_data_start_recover_request
{
	ndmp_pval		env<>;
	ndmp_name_v3    nlist<>;
	string			bu_type<>;

};
#endif

struct ndmp_data_start_recover_reply
{
	ndmp_error	error;
};

/* NDMP_DATA_ABORT */
/* no request arguments */
struct ndmp_data_abort_reply
{
	ndmp_error	error;
};

/* NDMP_DATA_STOP */
/* no request arguments */
struct ndmp_data_stop_reply
{
	ndmp_error	error;
};

/* NDMP_DATA_CONTINUE */
/* no request arguments */
struct ndmp_data_continue_reply
{
	ndmp_error	error;
};

/* NDMP_DATA_GET_ENV */
/* no request arguments */
struct ndmp_data_get_env_reply
{
	ndmp_error	error;
	ndmp_pval	env<>;
};

/* NDMP_NOTIFY_PAUSED */
struct ndmp_notify_paused_request
{
	ndmp_data_pause_reason	reason;
	ndmp_u_quad				resvd1;
};
/* No reply */

/* NDMP_NOTIFY_HALTED = NDMP_NOTIFY_DATA_HALTED */
struct ndmp_notify_halted_request
{
	ndmp_data_halt_reason	reason;
	string					text_reason<>;
};

#ifdef VER > 1
struct ndmp_notify_data_halted_request
{
	ndmp_data_halt_reason_v2	reason;
	string						text_reason<>;
};
#endif

/* No reply */

/********************************/
/* NOTIFY INTERFACE			    */
/********************************/
/* NDMP_NOTIFY_CONNECTED */
enum ndmp_connect_reason
{
	NDMP_CONNECTED,		/* Connect sucessfully */
	NDMP_SHUTDOWN,		/* Connection shutdown */
	NDMP_REFUSED		/* reach the maximum number of connections */
};

struct ndmp_notify_connected_request
{
	ndmp_connect_reason	reason;
	u_short				protocol_version;
	string				text_reason<>;
};

#if VER > 1
/* NOTIFY_MOVER_PAUSED */
struct ndmp_notify_mover_paused_request
{
	ndmp_mover_pause_reason	reason;
	ndmp_u_quad				seek_position;
};
/* No reply */

/* NOTIFY_MOVER_HALTED */
struct ndmp_notify_mover_halted_request
{
	ndmp_data_halt_reason	reason;
};
/* No reply */

/* NOTIFY_DATA_READ */
struct ndmp_notify_data_read_request
{
	ndmp_u_quad	offset;
	ndmp_u_quad	length;
};
/* No reply */
#endif

/********************************/
/* LOG INTERFACE				*/
/********************************/
/* NDMP_LOG_LOG */
struct ndmp_log_log_request
{
	string	entry<>;
};
/* No reply */

/* LOG_DEBUG */
enum ndmp_debug_level
{
	NDMP_DBG_USER_INFO,
	NDMP_DBG_USER_SUMMARY,
	NDMP_DBG_USER_DETAIL,
	NDMP_DBG_DIAG_INFO,
	NDMP_DBG_DIAG_SUMMARY,
	NDMP_DBG_DIAG_DETAIL,
	NDMP_DBG_PROG_INFO,
	NDMP_DBG_PROG_SUMMARY,
	NDMP_DBG_PROG_DETAIL
};

struct ndmp_log_debug_request
{
	ndmp_debug_level	level;
	string				message<>;
};
/* No reply */

/* NDMP_LOG_FILE */
struct ndmp_log_file_request
{
	string		name<>;
	u_short		ssid;
	ndmp_error	error;
};
/* No reply */

/********************************/
/* File History INTERFACE	    */
/********************************/
/* NDMP_FH_ADD_UNIX */
typedef string ndmp_unix_path<>;
enum ndmp_unix_file_type
{
	_NDMP_FILE_DIR,
	_NDMP_FILE_FIFO,
	_NDMP_FILE_CSPEC,
	_NDMP_FILE_BSPEC,
	_NDMP_FILE_REG,
	_NDMP_FILE_SLINK,
	_NDMP_FILE_SOCK
};

struct ndmp_unix_file_stat
{
	ndmp_unix_file_type	ftype;
	u_long				mtime;
	u_long				atime;
	u_long				ctime;
	u_long				uid;
	u_long				gid;
	u_long				mode;
	ndmp_u_quad			size;
	ndmp_u_quad			fh_info;
};

struct ndmp_fh_unix_path
{
	ndmp_unix_path		name;
	ndmp_unix_file_stat	fstat;
};

struct ndmp_fh_add_unix_path_request
{
	ndmp_fh_unix_path	paths<>;
};
/* No reply */

/* NDMP_FH_ADD_UNIX_DIR */
struct ndmp_fh_unix_dir
{
	ndmp_unix_path	name;
	u_long			node;
	u_long			parent;
};

struct ndmp_fh_add_unix_dir_request
{
	ndmp_fh_unix_dir	dirs<>;
};
/* No reply */

struct ndmp_fh_unix_node
{
	ndmp_unix_file_stat	fstat;
	u_long				node;
};

struct ndmp_fh_add_unix_node_request
{
	ndmp_fh_unix_node	nodes<>;
};

struct ndmp_ipv6_addr 
{ 
	opaque	__u6_addr8[16];
}; 

enum ndmp_addr_type
{
	NDMP_ADDR_LOCAL    = 0,
	NDMP_ADDR_TCP      = 1,
	NDMP_ADDR_FC       = 2, 	/* NDMP V2,3 */
	NDMP_ADDR_IPC      = 3,
	NDMP_ADDR_TCP_IPV6 = 0x20510004
};

struct ndmp_ipc_addr_v3
{
	opaque comm_data<>;
};


struct ndmp_tcp_addr_v4
{
	u_long       ip_addr;
	u_short      port;
	ndmp_pval    addr_env<>;
};

struct ndmp_tcp_ipv6_addr_v4
{
	ndmp_ipv6_addr       ipv6_addr;
	u_short      port;
	ndmp_pval    addr_env<>;
};

union ndmp_addr_v4
switch (ndmp_addr_type addr_type)
{
	case NDMP_ADDR_LOCAL:
		void;
	case NDMP_ADDR_TCP:
		ndmp_tcp_addr_v4  tcp_addr<>;
	case NDMP_ADDR_IPC:
		ndmp_ipc_addr_v3  ipc_addr;
	case NDMP_ADDR_TCP_IPV6:
		ndmp_tcp_ipv6_addr_v4 tcp_ipv6_addr<>;
	default:
	  void;
};

struct ndmp_data_connect_request
{
	ndmp_addr_v4   addr;
};

struct ndmp_data_connect_reply
{
	ndmp_error	error;
};

struct ndmp_data_listen_request
{
	ndmp_addr_type addr_type;
};

struct ndmp_data_listen_reply
{
	ndmp_error error;
	ndmp_addr_v4 addr;
};

/* NDMP_LOG_MESSAGE */
enum ndmp_log_type
{
	NDMP_LOG_NORMAL  = 0,
	NDMP_LOG_DEBUG   = 1,
	NDMP_LOG_ERROR   = 2,
	NDMP_LOG_WARNING = 3
};

enum ndmp_has_associated_message
{
	NDMP_NO_ASSOCIATED_MESSAGE     = 0,
	NDMP_HAS_ASSOCIATED_MESSAGE    = 1
};

struct ndmp_log_message_request_v4
{
	ndmp_log_type      log_type;
	u_long             message_id;
	string             entry<>;
	ndmp_has_associated_message associated_message_valid;
	u_long             associated_message_sequence;
};

enum ndmp_data_halt_reason
{
	NDMP_DATA_HALT_NA             = 0,
	NDMP_DATA_HALT_SUCCESSFUL     = 1,
	NDMP_DATA_HALT_ABORTED        = 2,
	NDMP_DATA_HALT_INTERNAL_ERROR = 3,
	NDMP_DATA_HALT_CONNECT_ERROR  = 4
};

struct ndmp_notify_data_halted_request_v4
{
	ndmp_data_halt_reason   reason;
};

enum ndmp_recovery_status
{
	NDMP_RECOVERY_SUCCESSFUL                 = 0,
	NDMP_RECOVERY_FAILED_PERMISSION          = 1,
	NDMP_RECOVERY_FAILED_NOT_FOUND           = 2,
	NDMP_RECOVERY_FAILED_NO_DIRECTORY        = 3,
	NDMP_RECOVERY_FAILED_OUT_OF_MEMORY       = 4,
	NDMP_RECOVERY_FAILED_IO_ERROR            = 5,
	NDMP_RECOVERY_FAILED_UNDEFINED_ERROR     = 6,
	NDMP_RECOVERY_FAILED_FILE_PATH_EXISTS    = 7
};

struct ndmp_log_file_request_v4
{
	string                   name<>;
	ndmp_recovery_status     recovery_status;
};

enum ndmp_data_state
{
	NDMP_DATA_STATE_IDLE      = 0,
	NDMP_DATA_STATE_ACTIVE    = 1,
	NDMP_DATA_STATE_HALTED    = 2,
	NDMP_DATA_STATE_LISTEN    = 3, 		/* NDMP V3 */
	NDMP_DATA_STATE_CONNECTED = 4 		/* NDMP V3 */
};

struct ndmp_data_get_state_reply_v4
{
	u_long                    unsupported;
	ndmp_error                error;
	ndmp_data_operation       operation;
	ndmp_data_state           state;
	ndmp_data_halt_reason     halt_reason;
	ndmp_u_quad               bytes_processed;
	ndmp_u_quad               est_bytes_remain;
	u_long                    est_time_remain;
	ndmp_addr_v4              data_connection_addr;
	ndmp_u_quad               read_offset;
	ndmp_u_quad               read_length;
};

typedef string ndmp_path_v3<>;
struct ndmp_nt_path_v3
{
	ndmp_path_v3	nt_path;
	ndmp_path_v3	dos_path;
};

union ndmp_file_name_v3 switch (ndmp_fs_type fs_type)
{
	case NDMP_FS_UNIX:
		ndmp_path_v3		unix_name;
	case NDMP_FS_NT:
		ndmp_nt_path_v3	nt_name;
	default:
		ndmp_path_v3		other_name;
};

struct ndmp_dir_v3
{
	ndmp_file_name_v3	names<>;
	ndmp_u_quad		node;
	ndmp_u_quad		parent;
};

struct ndmp_fh_add_dir_request_v3
{
	ndmp_dir_v3	dirs<>;
};

enum ndmp_fs_type
{
	NDMP_FS_UNIX   = 0,
	NDMP_FS_NT     = 1,
	NDMP_FS_OTHER  = 2
};

enum ndmp_file_type
{
	NDMP_FILE_DIR      = 0,
	NDMP_FILE_FIFO     = 1,
	NDMP_FILE_CSPEC    = 2,
	NDMP_FILE_BSPEC    = 3,
	NDMP_FILE_REG      = 4,
	NDMP_FILE_SLINK    = 5,
	NDMP_FILE_SOCK     = 6,
	NDMP_FILE_REGISTRY = 7,
	NDMP_FILE_OTHER    = 8
};

struct ndmp_file_stat_v3
{
	u_long			invalid;
	ndmp_fs_type		fs_type;
	ndmp_file_type		ftype;
	u_long			mtime;
	u_long			atime;
	u_long			ctime;
	u_long			owner; /* uid for UNIX, owner for NT */
	u_long			group; /* gid for UNIX, NA for NT */
	u_long			fattr; /* mode for UNIX, fattr for NT */
	ndmp_u_quad		size;
	u_long			links;
};

struct ndmp_node_v3
{
	ndmp_file_stat_v3	stats<>;
	ndmp_u_quad		node;
	ndmp_u_quad		fh_info;
};

struct ndmp_fh_add_node_request_v3
{
	ndmp_node_v3	nodes<>;
};
/* no reply arguments */

/* No reply */
