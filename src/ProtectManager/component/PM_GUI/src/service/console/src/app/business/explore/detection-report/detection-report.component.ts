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
  OnInit,
  AfterViewInit,
  ViewChild,
  TemplateRef
} from '@angular/core';
import { PaginatorComponent, SearchComponent } from '@iux/live';
import {
  CommonConsts,
  DetectReportAPIService,
  I18NService,
  MODAL_COMMON,
  WarningMessageService
} from 'app/shared';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import {
  assign,
  cloneDeep,
  each,
  eq,
  filter,
  includes,
  isEmpty,
  isNumber,
  isString,
  map,
  remove,
  size
} from 'lodash';
import { AddReportComponent } from './add-report/add-report.component';

@Component({
  selector: 'aui-detection-report',
  templateUrl: './detection-report.component.html',
  styleUrls: ['./detection-report.component.less']
})
export class DetectionReportComponent implements OnInit, AfterViewInit {
  listData = [];
  uuidArr = [];
  selection = [];
  page = 1;
  pageSize = CommonConsts.PAGE_SIZE;
  total: number;

  totalInfected = 0;
  totalUnInfected = 0;
  totalReport = 0;

  @ViewChild('searchCmpt', { static: true })
  searchCmpt: SearchComponent;
  @ViewChild('paginator', { static: true })
  paginator: PaginatorComponent;
  constructor(
    private detectReportApi: DetectReportAPIService,
    private i18n: I18NService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService
  ) {}

  ngOnInit() {
    this.getReportSummary();
  }

  getReportSummary(recordsTemp?: any[], startPage?: number) {
    this.detectReportApi
      .ListQueryDetectReport({
        startPage: startPage || CommonConsts.PAGE_START + 1,
        pageSize: CommonConsts.PAGE_SIZE_MAX
      })
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START + 1;
        }
        startPage++;
        recordsTemp = [...recordsTemp, ...res.records];
        if (
          startPage ===
            Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE_MAX) + 1 ||
          res.totalCount === 0
        ) {
          this.totalReport = size(recordsTemp);
          this.totalInfected = size(filter(recordsTemp, item => item.infected));
          this.totalUnInfected = this.totalReport - this.totalInfected;
          return;
        }
        this.getReportSummary(recordsTemp, startPage);
      });
  }

  ngAfterViewInit() {
    this.loadData(true);
  }

  loadData(initUUidArr = false, _params = {}) {
    this.detectReportApi
      .ListQueryDetectReport({
        startPage: this.page,
        pageSize: this.pageSize,
        ..._params
      })
      .subscribe(res => {
        if (initUUidArr && size(res.records)) {
          this.uuidArr = map(res.records, 'uuid');
        }
        this.listData = this.initProgress(res.records);
        this.total = res.totalCount;
        this.selection = [];
        this.getReportSummary();
      });
  }
  pageChange(data) {
    this.page = data.pageIndex + 1;
    this.pageSize = data.pageSize;
    this.loadData(true, {
      name: isEmpty(this.searchCmpt.value) ? void 0 : this.searchCmpt.value
    });
  }
  initProgress(list: any[]) {
    const listData = cloneDeep(list);
    each(listData, item => {
      if (!includes(this.uuidArr, item.uuid)) {
        item.showProgress = true;
        item.progressValue = 0;

        // 规定时间内加载完成
        item.timer = setInterval(() => {
          item.progressValue += 2;
          if (item.progressValue >= 100) {
            item.progressValue = 100;
            item.showProgress = false;
            this.uuidArr.push(item.uuid);
            clearInterval(item.timer);
          }
        }, Math.ceil(2000 / 100));
      }
    });
    return listData;
  }

  search(val) {
    this.page = 1;
    this.paginator.setSelected(0);
    this.loadData(false, { name: val });
  }
  addReported() {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-storage-modal',
        lvWidth: MODAL_COMMON.normalWidth + 100,
        lvHeader: this.i18n.get('common_add_label'),
        lvContent: AddReportComponent,
        lvOkDisabled: true,
        lvAfterOpen: modal => {
          const content = modal.getContentComponent() as AddReportComponent;
          content.formGroup.statusChanges.subscribe(status => {
            modal.getInstance().lvOkDisabled = status !== 'VALID';
          });
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as AddReportComponent;

            content.onOK().subscribe({
              next: () => {
                resolve(true);
                this.loadData();
              },
              error: () => resolve(false)
            });
          });
        }
      })
    );
  }
  selectChange(val: object | string) {
    if (isString(val)) {
      remove(this.selection, item => eq(item.uuid, val));
      return;
    }
    this.selection.push(val);
  }

  remove(selection) {
    this.warningMessageService.create({
      content: this.i18n.get('insight_delete_report_label'),
      onOK: () => {
        this.detectReportApi
          .DeleteDetectReportById({
            reportIdList: map(selection, 'uuid')
          })
          .subscribe(() => {
            each(selection, item => {
              remove(this.uuidArr, i => eq(i, item.uuid));
            });
            this.loadData();
          });
      }
    });
  }

  getDetail(data) {
    window.open('/console/#/report-detail/' + data.uuid);
  }
}
