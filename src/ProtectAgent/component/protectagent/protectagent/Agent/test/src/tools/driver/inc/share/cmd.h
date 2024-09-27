#ifndef _OM_CMD_H
#define _OM_CMD_H




#ifdef _MSC_VER
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef unsigned long long uint64_t;
typedef long long int64_t;

#pragma warning(disable : 4200)
#endif


#ifndef VM_ID_LEN
#define VM_ID_LEN      (16)
#endif

#ifndef VOL_ID_LEN
#define VOL_ID_LEN     (16)
#endif

#ifndef TOKEN_ID_LEN
#define TOKEN_ID_LEN   (16)
#endif

#pragma pack(push)
#pragma pack(1)
typedef struct _DPP_HEADER
{
	uint32_t magic;
	uint16_t cmd_type;
	uint16_t flags;
	uint64_t sequence_num;
	uint32_t body_len;
	uint32_t reserved;
}DPP_HEADER, *PDPP_HEADER;


typedef struct _DPP_SESSION_LOGIN_AUX
{
    uint64_t max_dataset_size;
    uint64_t dataset_id_sent;
    uint64_t dataset_id_done;
    uint8_t  token_id[TOKEN_ID_LEN];
}DPP_SESSION_LOGIN_AUX, *PDPP_SESSION_LOGIN_AUX;
#define DPP_SESSION_LOGIN_AUX_LEN			(sizeof(DPP_SESSION_LOGIN_AUX))

typedef struct _DPP_SESSION_LOGIN_ACK_AUX
{
	uint64_t max_dataset_size;
	uint64_t dataset_id_sent;
	uint64_t dataset_id_done;
	uint32_t buf_credit;
}DPP_SESSION_LOGIN_ACK_AUX, *PDPP_SESSION_LOGIN_ACK_AUX;
#define DPP_SESSION_LOGIN_ACK_AUX_LEN			(sizeof(DPP_SESSION_LOGIN_ACK_AUX))

#define DPP_SESSION_LOGIN_AUTH_LEN			(0)

typedef struct _DPP_SESSION_LOGIN
{
	uint8_t vm_id[VM_ID_LEN];
	uint8_t version;
	uint8_t reserved[3];
	uint16_t aux_login_len;
	uint16_t auth_len;
	uint8_t payload[0];
}DPP_SESSION_LOGIN, *PDPP_SESSION_LOGIN;
#define DPP_SESSION_LOGIN_LEN					(sizeof(DPP_SESSION_LOGIN) + DPP_SESSION_LOGIN_AUX_LEN + DPP_SESSION_LOGIN_AUTH_LEN)
#define DPP_SESSION_LOGIN_ACK_LEN			(sizeof(DPP_SESSION_LOGIN) + DPP_SESSION_LOGIN_ACK_AUX_LEN + DPP_SESSION_LOGIN_AUTH_LEN)

typedef struct _DPP_CREDIT_BUFFER
{
	uint8_t vm_id[VM_ID_LEN];
	uint32_t buf_credit;
}DPP_CREDIT_BUFFER, *PDPP_CREDIT_BUFFER;

typedef struct _DPP_VOL_VEC_ENTRY
{
	uint8_t vol_id[VOL_ID_LEN];
	uint64_t vol_offset;
	uint64_t seg_size;
}DPP_VOL_VEC_ENTRY, *PDPP_VOL_VEC_ENTRY;

typedef struct _DPP_RESYNCSET_START
{
	uint8_t vm_id[VM_ID_LEN];
	uint64_t resyncset_id;
	uint32_t num_vols;
	uint8_t reserved[4];
	DPP_VOL_VEC_ENTRY vol_entry[0];
}DPP_RESYNCSET_START, *PDPP_RESYNCSET_START;

typedef struct _DPP_DATASET_START
{
	uint8_t vm_id[VM_ID_LEN];
	uint64_t dataset_id;
	uint32_t dpp_type;
}DPP_DATASET_START, *PDPP_DATASET_START;

typedef struct _DPP_DATASET_DONE
{
	uint8_t vm_id[VM_ID_LEN];
	uint64_t dataset_id;
}DPP_DATASET_DONE, *PDPP_DATASET_DONE;

typedef struct _DPP_DATA
{
	uint8_t vol_id[VOL_ID_LEN];
	uint64_t vol_offset;
	uint32_t data_size;
	uint8_t data[0];
}DPP_DATA, *PDPP_DATA;


typedef struct _DPP_ATTENTION
{
	uint8_t vm_id[VM_ID_LEN];
	uint32_t operation;
	uint32_t payload_len;
	uint8_t payload[0];
}DPP_ATTENTION, *PDPP_ATTENTION;

typedef struct _DPP_ATTENTION_PAYLOAD_ALERT
{
	uint32_t level;
	uint32_t description;
}DPP_ATTENTION_PAYLOAD_ALERT, *PDPP_ATTENTION_PAYLOAD_ALERT;

typedef struct _DPP_ATTENTION_PAYLOAD_ACTIVITY
{
	uint64_t cbt_backlog;
	uint64_t resync_remaining;
}DPP_ATTENTION_PAYLOAD_ACTIVITY, *PDPP_ATTENTION_PAYLOAD_ACTIVITY;

typedef struct _DPP_FLUSH
{
	uint8_t vm_id[VM_ID_LEN];
}DPP_FLUSH, *PDPP_FLUSH;


typedef struct IOMirrorCmd
{
	DPP_HEADER   header;
	uint8_t data[0];
} IOMirrorCmd;
#pragma pack(pop)

#define DPP_MAGIC		(0x72634552)

enum {DPP_TYPE_SESSION_LOGIN = 0, DPP_TYPE_SESSION_LOGIN_ACK = 1, DPP_TYPE_CREDIT = 11, DPP_TYPE_RESYNCSET_START = 20, DPP_TYPE_RESYNCSET_DONE = 21, DPP_TYPE_DATASET_START = 22, DPP_TYPE_DATASET_DONE = 25, DPP_TYPE_DATA = 26, DPP_TYPE_HEARTBEAT = 31, DPP_TYPE_HEARTBEAT_ACK = 32, DPP_TYPE_ATTENTION = 33, DPP_TYPE_FLUSH = 35};
enum {DPP_FLAG_SOURCE_IOMIRROR = 1, DPP_FLAG_SOURCE_SOMA = 2, DPP_FLAG_DEST_SOMA = 0x10, DPP_FLAG_ATTENTION = 0x200, DPP_FLAG_FAIL = 0x8000};
enum {DPP_DATASET_TYPE_NORMAL = 0, DPP_DATASET_TYPE_CBT};
enum {DPP_ATTENTION_OPERATION_DISCOVERY = 0, DPP_ATTENTION_OPERATION_ALERT, DPP_ATTENTION_OPERATION_ACTIVITY};
enum {DPP_ATTENTION_PAYLOAD_ALERT_LEVEL_FATAL = 0, DPP_ATTENTION_PAYLOAD_ALERT_LEVEL_ERROR, DPP_ATTENTION_PAYLOAD_ALERT_LEVEL_WARNING, DPP_ATTENTION_PAYLOAD_ALERT_LEVEL_INFO, DPP_ATTENTION_PAYLOAD_ALERT_LEVEL_DEBUG};
enum {DPP_ATTENTION_PAYLOAD_ALERT_DESCRIPTION_NETWORK_RESET = 0 };

#endif

