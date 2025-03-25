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
import {
  Component,
  EventEmitter,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  JOB_ORIGIN_TYPE,
  JobAPIService,
  WarningMessageService
} from 'app/shared';
import { assign, includes, map, reject, size } from 'lodash';
import { Observable, Observer } from 'rxjs';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { TableCols, TableConfig } from 'app/shared/components/pro-table';

@Component({
  selector: 'aui-batch-retry',
  templateUrl: './batch-retry.component.html',
  styleUrls: ['./batch-retry.component.css']
})
export class BatchRetryComponent implements OnInit {
  jobOriginType = JOB_ORIGIN_TYPE;
  selections;
  invalidEmitter = new EventEmitter<boolean>();
  tableConfig: TableConfig;
  tableData = {
    data: [],
    total: 0
  };
  statusArr = map(
    reject(this.dataMapService.toArray('Job_status'), item =>
      includes(
        [
          DataMap.Job_status.running.value,
          DataMap.Job_status.initialization.value,
          DataMap.Job_status.pending.value,
          DataMap.Job_status.aborting.value,
          DataMap.Job_status.dispatching.value,
          DataMap.Job_status.redispatch.value,
          DataMap.Job_status.dispatch_failed.value
        ],
        item.value
      )
    ),
    'value'
  );
  @ViewChild('warningWindowTpl', { static: true })
  warningWindowTpl: TemplateRef<void>;

  constructor(
    private jobApiService: JobAPIService,
    public i18n: I18NService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initTable();
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'jobId',
        name: this.i18n.get('common_job_id_label')
      },
      {
        key: 'targetLocation',
        name: this.i18n.get('protection_restore_target_label')
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        compareWith: 'jobId',
        columns: cols
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showTotal: true
      }
    };
  }

  updateSelections(res) {
    this.selections = res;
    this.invalidEmitter.emit(!size(this.selections));
  }

  onOK() {
    this.tableData = {
      data: this.selections,
      total: size(this.selections)
    };
    return new Observable((observer: Observer<void>) => {
      this.warningMessageService.create({
        content: this.warningWindowTpl,
        onOK: () => {
          this.batchOperateService.selfGetResults(
            item =>
              this.jobApiService.retryJobUsingPost({
                jobId: item.jobId
              }),
            this.selections.map(item => assign(item, { name: item.jobId })),
            () => {
              observer.next();
              observer.complete();
            }
          );
        },
        onCancel: () => {
          observer.error(null);
          observer.complete();
        },
        lvAfterClose: result => {
          if (result && result.trigger === 'close') {
            observer.error(null);
            observer.complete();
          }
        }
      });
    });
  }
}
