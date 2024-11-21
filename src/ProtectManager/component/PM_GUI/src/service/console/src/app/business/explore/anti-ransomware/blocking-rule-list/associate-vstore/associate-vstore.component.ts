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
  ChangeDetectorRef,
  Component,
  OnInit,
  ViewChild
} from '@angular/core';
import {
  I18NService,
  FileExtensionFilterManagementService,
  WarningMessageService,
  WhiteListManagementService
} from 'app/shared';
import {
  TableConfig,
  TableData,
  ProTableComponent,
  TableCols,
  Filters
} from 'app/shared/components/pro-table';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  size,
  isEmpty,
  assign,
  map,
  find,
  isUndefined,
  cloneDeep
} from 'lodash';
import { Subject, Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-associate-vstore',
  templateUrl: './associate-vstore.component.html',
  styleUrls: ['./associate-vstore.component.less']
})
export class AssociateVstoreComponent implements OnInit, AfterViewInit {
  isWhiteList;
  ids;
  isAssciate = true;
  isDetail = false;
  vstoreInfos = [];
  extensions = [];
  selectionData = [];
  assciateVstores = [];
  tableConfig: TableConfig;
  tableData: TableData;
  valid$ = new Subject<boolean>();
  helpLabel = this.i18n.get('explore_select_associate_vstore_tip_label');

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private virtualScroll: VirtualScrollService,
    private warningMessageService: WarningMessageService,
    private fileExtensionFilterManagementService: FileExtensionFilterManagementService,
    private whiteListManagementService: WhiteListManagementService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'vstoreName',
        name: this.i18n.get('common_tenant_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      }
    ];
    if (this.isDetail) {
      this.tableConfig = {
        table: {
          async: false,
          columns: cols,
          colDisplayControl: false,
          scrollFixed: true
        },
        pagination: {
          winTablePagination: true,
          showPageSizeOptions: false,
          mode: 'simple'
        }
      };
      this.tableData = {
        data: this.vstoreInfos,
        total: size(this.vstoreInfos)
      };
    } else if (this.isAssciate) {
      this.tableConfig = {
        table: {
          columns: cols,
          compareWith: 'vstoreId',
          showLoading: false,
          colDisplayControl: false,
          rows: {
            selectionMode: 'multiple',
            selectionTrigger: 'selector',
            showSelector: true
          },
          scrollFixed: true,
          fetchData: (filter: Filters) => {
            this.getData(filter);
          },
          selectionChange: selection => {
            this.selectionData = selection;
            this.valid$.next(!!size(this.selectionData));
          }
        },
        pagination: {
          winTablePagination: true,
          showPageSizeOptions: false,
          mode: 'simple'
        }
      };
      if (size(this.extensions) === 1) {
        setTimeout(() => {
          this.dataTable.setSelections(cloneDeep(this.assciateVstores));
        }, 500);
      }
    } else {
      this.tableConfig = {
        table: {
          async: false,
          columns: cols,
          compareWith: 'vstoreId',
          showLoading: false,
          colDisplayControl: false,
          rows: {
            selectionMode: 'multiple',
            selectionTrigger: 'selector',
            showSelector: true
          },
          virtualScroll: true,
          scrollFixed: true,
          scroll: this.virtualScroll.scrollParam,
          selectionChange: (selection, renderSelection) => {
            this.selectionData = selection;
            this.valid$.next(!!size(this.selectionData));
          }
        },
        pagination: {
          mode: 'simple',
          showPageSizeOptions: false,
          winTablePagination: true
        }
      };
      this.tableData = {
        data: this.assciateVstores,
        total: size(this.assciateVstores)
      };
      this.helpLabel = this.i18n.get(
        'explore_select_disassociate_vstore_tip_label'
      );
    }
  }

  getData(filters?: Filters) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize
    };
    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      assign(params, { vstoreName: conditions.vstoreName });
    }
    this.fileExtensionFilterManagementService
      .getVstoreInfoUsingGET(params)
      .subscribe(res => {
        if (size(this.extensions) === 1) {
          res.records.filter(item =>
            isUndefined(
              find(this.assciateVstores, {
                vstoreId: item.vstoreId
              })
            )
              ? item
              : assign(item, { disabled: true })
          );
        }
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
        this.cdr.detectChanges();
      });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (!this.isAssciate) {
        this.warningMessageService.create({
          content: this.isWhiteList
            ? this.i18n.get(
                'explore_disassociate_whitelist_file_extension_label',
                [map(this.selectionData, 'vstoreName').toString()]
              )
            : this.i18n.get(
                'explore_disassociate_blocking_file_extension_label',
                [map(this.selectionData, 'vstoreName').toString()]
              ),
          onCancel: () => {
            observer.error(null);
            observer.complete();
          },
          onOK: () => {
            if (this.isWhiteList) {
              this.whiteListManagementService
                .deleteWhiteListAssociationUsingDELETE({
                  deleteWhiteListAssociationRequest: {
                    ids: this.ids,
                    vstoreIds: map(this.selectionData, 'vstoreId')
                  }
                })
                .subscribe(
                  res => {
                    observer.next();
                    observer.complete();
                  },
                  err => {
                    observer.error(err);
                    observer.complete();
                  }
                );
            } else {
              this.fileExtensionFilterManagementService
                .deleteFileExtensionFilterUsingGet({
                  importUserSuffixRequest: {
                    extensions: this.extensions,
                    vstoreIds: map(this.selectionData, 'vstoreId')
                  }
                })
                .subscribe(
                  res => {
                    observer.next();
                    observer.complete();
                  },
                  err => {
                    observer.error(err);
                    observer.complete();
                  }
                );
            }
          }
        });
        return;
      } else {
        if (this.isWhiteList) {
          this.warningMessageService.create({
            content: this.i18n.get('explore_whitelist_associate_vstore_label', [
              map(this.selectionData, 'vstoreName').toString()
            ]),
            onCancel: () => {
              observer.error(null);
              observer.complete();
            },
            onOK: () => {
              this.whiteListManagementService
                .createWhiteListAssociationUsingPOST({
                  createWhiteListAssociationRequest: {
                    ids: this.ids,
                    vstoreIds: map(this.selectionData, 'vstoreId')
                  }
                })
                .subscribe(
                  res => {
                    observer.next();
                    observer.complete();
                  },
                  err => {
                    observer.error(err);
                    observer.complete();
                  }
                );
              return;
            }
          });
        } else {
          this.fileExtensionFilterManagementService
            .importFileExtensionFilterUsingPOST({
              importUserSuffixRequest: {
                extensions: this.extensions,
                vstoreIds: map(this.selectionData, 'vstoreId')
              }
            })
            .subscribe(
              res => {
                observer.next();
                observer.complete();
              },
              err => {
                observer.error(err);
                observer.complete();
              }
            );
          return;
        }
      }
    });
  }
}
