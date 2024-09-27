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
  AfterViewInit,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';
import {
  CommonConsts,
  DataMapService,
  I18NService,
  IODETECTPOLICYService,
  IODETECTWHITELISTService,
  WarningMessageService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, isEmpty, isNumber, map, size } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-associate-policy',
  templateUrl: './associate-policy.component.html',
  styleUrls: ['./associate-policy.component.less']
})
export class AssociatePolicyComponent implements OnInit, AfterViewInit {
  rowData;
  isDisassociated;
  isDetail;

  tableConfig: TableConfig;
  tableData: TableData;
  selectionData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('periodTpl', { static: true })
  periodTpl: TemplateRef<any>;
  @ViewChild('retentionTol', { static: true })
  retentionTol: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private warningMessageService: WarningMessageService,
    private ioDetectPolicyService: IODETECTPOLICYService,
    private ioDetectWhitelistService: IODETECTWHITELISTService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
    this.getAssociatedPolicy();
  }

  ngOnInit(): void {
    this.initTableConfig();
  }

  getOkDisabled() {
    this.modal.getInstance().lvOkDisabled = !size(this.selectionData);
  }

  initTableConfig() {
    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: [
          {
            key: 'name',
            name: this.i18n.get('explore_policy_name_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'retentionDuration',
            name: this.i18n.get('explore_snapshot_lock_time_label'),
            cellRender: this.retentionTol,
            sort: true
          },
          {
            key: 'isIoEnhancedEnabled',
            name: this.i18n.get('explore_alarm_analysis_label'),
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              options: this.dataMapService.toArray('switchStatus')
            },
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('switchStatus')
            }
          },
          {
            key: 'isHoneypotDetectEnable',
            name: this.i18n.get('explore_decoy_detection_status_label'),
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              options: this.dataMapService.toArray('switchStatus')
            },
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('switchStatus')
            }
          },
          {
            key: 'period',
            name: this.i18n.get('explore_decoy_update_frequency_label'),
            sort: true,
            cellRender: this.periodTpl
          }
        ],
        rows: this.isDetail
          ? null
          : {
              selectionMode: 'multiple',
              selectionTrigger: 'selector',
              showSelector: true
            },
        scrollFixed: true,
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionData = selection;
          this.getOkDisabled();
        },
        fetchData: (filters: Filters) => this.getData(filters),
        trackByFn: (_, item) => {
          return item.id;
        }
      },
      pagination: {
        winTablePagination: true,
        showPageSizeOptions: false,
        mode: 'simple',
        pageSize: CommonConsts.PAGE_SIZE_SMALL
      }
    };
  }

  getAssociatedPolicy(recordsTemp?: any[], startPage?: number) {
    if (this.isDetail || this.isDisassociated) {
      return;
    }
    const extParams = {
      pageNum: startPage || CommonConsts.PAGE_START,
      pageSize: 100,
      whitelistIds: map(this.rowData, 'id')
    };
    this.ioDetectPolicyService
      .pageQueryIoDetectPolicy(extParams)
      .subscribe(res => {
        if (!recordsTemp) {
          recordsTemp = [];
        }
        if (!isNumber(startPage)) {
          startPage = CommonConsts.PAGE_START;
        }
        recordsTemp = [...recordsTemp, ...res.records];
        const page = Math.floor(res.totalCount / 100);
        if (startPage === page || res.totalCount === 0) {
          this.selectionData = recordsTemp;
          this.dataTable.setSelections(this.selectionData);
          this.getOkDisabled();
          return;
        }
        startPage++;
        this.getAssociatedPolicy(recordsTemp, startPage);
      });
  }

  getData(filters: Filters) {
    const params = {
      pageNum: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    if (this.isDetail) {
      assign(params, {
        whitelistIds: [this.rowData?.id]
      });
    }

    if (this.isDisassociated) {
      assign(params, {
        whitelistIds: map(this.rowData, 'id')
      });
    }

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (conditions.isHoneypotDetectEnable) {
        assign(params, {
          honeypotDetectStatus: conditions.isHoneypotDetectEnable
        });
        delete conditions.isHoneypotDetectEnable;
      }
      if (conditions.isIoEnhancedEnabled) {
        assign(params, {
          ioEnhancedStatus: conditions.isIoEnhancedEnabled
        });
        delete conditions.isIoEnhancedEnabled;
      }
      assign(params, conditions);
    }

    if (!isEmpty(filters.sort) && !isEmpty(filters.sort.key)) {
      assign(params, {
        orderType: filters.sort.direction,
        orderBy: filters.sort.key
      });
    }

    this.ioDetectPolicyService
      .pageQueryIoDetectPolicy(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  deleteWhiteList(policyIds, whitelistIds, observer) {
    this.warningMessageService.create({
      content: this.i18n.get('explore_disassociate_warn_label', [
        map(this.selectionData, 'name').join(this.i18n.isEn ? ',' : '，')
      ]),
      onOK: () => {
        this.ioDetectWhitelistService
          .deleteWhiteListAssociation({
            delWhitelistAssocReq: { policyIds, whitelistIds }
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            () => {
              observer.error(null);
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
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const policyIds = map(this.selectionData, 'id');
      const whitelistIds = map(this.rowData, 'id');
      if (this.isDisassociated) {
        this.deleteWhiteList(policyIds, whitelistIds, observer);
      } else {
        this.ioDetectWhitelistService
          .createWhiteListAssociation({
            createWhitelistAssocReq: { policyIds, whitelistIds }
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            () => {
              observer.error(null);
              observer.complete();
            }
          );
      }
    });
  }
}
