data = {"uuid": "",
        "name": "test001",
        "type": 1,
        "application": "vim.VirtualMachine",
        "policy_list": [
            {
                "uuid": "",
                "name": "全量01",
                "type": "backup",
                "action": "full",
                "retention": {
                    "retention_type": 2,
                    "retention_duration": 1,
                    "duration_unit": "y"
                },
                "schedule": {
                    "trigger": 4,
                    "window_start": "00:00:00",
                    "window_end": "00:00:00",
                    "trigger_action": "year",
                    "days_of_year": "2024-04-16"
                },
                "ext_parameters": {
                    "fine_grained_restore": False,
                    "ensure_consistency_backup": True,
                    "alarm_after_failure": True,
                    "source_deduplication": False,
                    "ensure_storage_layer_backup": False,
                    "qos_id": "",
                    "ensure_deleted_data": False,
                    "ensure_specifies_transfer_mode": False,
                    "specifies_transfer_mode": "",
                    "auto_retry": True,
                    "auto_retry_times": 3,
                    "auto_retry_wait_minutes": 5,
                    "storage_info": {}
                }
            }
        ]
        }
