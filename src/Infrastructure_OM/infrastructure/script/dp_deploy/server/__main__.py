#!/usr/bin/env python
# _*_ coding:utf-8 _*_
# /*$$$!!Warning: Huawei key information asset. No spread without permission.$$$*/
# /*CODEMARK:rLr9evwLcHFK0ocH1vI5dv79JXrhoRrQ3pUSCsHdOM/puzvFwKILKNePe4bG/B98pz4V5doHi56R
# 0f3JfrYASX9dpcZzoFk8wkkpq5MAD4QH7vM9CsfnwL0y/c+rHjXKrQ8MVGc+XqcBtj8Yza8MaIQ4
# /ST8U6y6HofoEwJrgIV1PqdwKuSGnbCr26ut0vTK/X7j0meFoplM2zB9Q+wy44RIjvsz2jXnpNMi
# 0AoM4CsvOo9RvT/lahddqlVxb7zV#*/
# /*$$$!!Warning: Deleting or modifying the preceding information is prohibited.$$$*/

import argparse

from server.e6000_main import e6000_main
from server.models.bridge import Bridge
from server.models.cert_manager_databackup import CertManagerDataBackup
from server.models.dpserver_databackup import DpServerDataBackup
from server.models.bridge import Context

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='dataprotect deployment server.')
    parser.add_argument('--address',
                        required=True,
                        help='external management ip.')

    parser.add_argument('--type',
                        required=True,
                        choices=["E6000", "DataBackup"],
                        help='device type')
    args = parser.parse_args()
    context = Context()

    if args.type == "E6000":
        e6000_main(args)
    elif args.type == "DataBackup":
        cert_manager = CertManagerDataBackup()
        dpsever = DpServerDataBackup(args.address)
        bridge = Bridge(cert_manager, dpsever)
        bridge.run(context)
