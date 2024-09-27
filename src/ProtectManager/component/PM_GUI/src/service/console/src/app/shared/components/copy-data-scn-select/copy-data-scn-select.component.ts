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
import { Component, OnInit, ViewChild } from '@angular/core';
import { CopiesService } from 'app/shared/api/services/copies.service';
import { RestoreService } from 'app/shared/services/restore.service';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { DatatableComponent } from '@iux/live';
import { Subject } from 'rxjs';
import { first } from 'lodash';

@Component({
  selector: 'aui-copy-data-scn-select',
  templateUrl: './copy-data-scn-select.component.html',
  styleUrls: ['./copy-data-scn-select.component.less']
})
export class CopyDataScnSelectComponent implements OnInit {
  scn;
  rowData;
  restoreType;
  tableData;
  @ViewChild('copyTable', { static: false }) copyTable: DatatableComponent;
  selection$ = new Subject<any>();

  constructor(
    private restoreService: RestoreService,
    private drawModalService: DrawModalService,
    private copiesApiService: CopiesService
  ) {}

  ngOnInit() {
    this.initData();
  }

  initData() {}

  selectionCopyRow(source) {
    this.copyTable.toggleSelection(source);
    this.selection$.next(this.copyTable.getSelection());
  }

  restore() {
    const selectedData = first(this.copyTable.getSelection());
    this.copiesApiService
      .queryResourcesV1CopiesGet({
        pageNo: 0,
        pageSize: 1,
        conditions: JSON.stringify({ uuid: selectedData.id })
      })
      .subscribe(res => {
        this.restoreService.restore({
          childResType: this.rowData.sub_type,
          copyData: {
            ...selectedData,
            uuid: selectedData.id,
            display_timestamp: selectedData.timestamp,
            dbUuid: this.rowData.uuid,
            resource_type: this.rowData.sub_type,
            resource_sub_type: this.rowData.sub_type,
            environment_os_type: this.rowData.environment_os_type,
            environment_uuid: this.rowData.environment_uuid,
            parent_uuid: this.rowData.parent_uuid,
            path: this.rowData.path,
            name: this.rowData.name,
            resource_id: this.rowData.uuid,
            resource_properties: JSON.stringify(this.rowData),
            properties: JSON.stringify({
              oracle_metadata: {
                ORACLEPARAM: {
                  db_name: this.rowData.name,
                  version: this.rowData.version
                }
              }
            }),
            scn: this.scn,
            rowCoyByScn: first(res.items)
          },
          restoreType: this.restoreType
        });
      });
  }
}
