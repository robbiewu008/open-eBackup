[Unit]
Description=DataProtect deployment server daemon
Wants=network-online.target

[Install]
WantedBy=multi-user.target

[Service]
Type=simple
ExecStart=/usr/bin/dpserver --address=${ADDRESS} --type=${TYPE}
KillMode=process
User=root
Group=root
TimeoutStartSec=0
Restart=always
RestartSec=5s