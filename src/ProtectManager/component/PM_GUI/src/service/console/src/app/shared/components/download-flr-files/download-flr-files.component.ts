/*
 * This file is a part of the open-eBackup project.
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at
 * http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) [2024] Huawei Technologies Co.,Ltd.
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 */
import { Component, OnInit } from '@angular/core';
import { DataMap } from 'app/shared';
import { ExportFilesService } from '../export-files/export-files.component';
import { assign } from 'lodash';

@Component({
  selector: 'aui-download-flr-files',
  templateUrl: './download-flr-files.component.html',
  styleUrls: ['./download-flr-files.component.less']
})
export class DownloadFlrFilesComponent implements OnInit {
  constructor(private exportFilesService: ExportFilesService) {}

  ngOnInit() {}

  getRequestId(paths, copyId, memberEsn?, tip?) {
    const params = { copyId, paths };
    if (memberEsn) {
      params['memberEsn'] = memberEsn;
    }
    const data = { params, type: DataMap.Export_Query_Type.copy.value };
    if (tip) {
      assign(data, { tip });
    }
    this.exportFilesService.create({ data });
  }
}
