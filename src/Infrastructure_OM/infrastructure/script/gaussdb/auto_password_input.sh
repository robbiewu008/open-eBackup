#!/bin/expect

set gaussdb_path [lindex $argv 0]
set datasync_path [lindex $argv 1]
set passwd [lindex $argv 2]
set type [lindex $argv 3]

cd $datasync_path
spawn java -jar DSS.jar -p config/cfg.ini -pwd $type
    expect "Please enter the password to be encrypted and press Enter to confirm!"
    send -- "${passwd}\r"
    expect eof

cd $gaussdb_path