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
import {
  ApiMultiClustersService,
  CommonConsts,
  I18NService,
  MODAL_COMMON,
  WarningMessageService
} from 'app/shared';
import {
  Filters,
  ProTableComponent,
  TableConfig
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, each, size } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { CancleWarnComponent } from './cancle-warn/cancle-warn.component';

@Component({
  selector: 'aui-cancle-auth-user',
  templateUrl: './cancle-auth-user.component.html',
  styleUrls: ['./cancle-auth-user.component.less']
})
export class CancleAuthUserComponent implements OnInit {
  rowItem;
  localCluster;
  userTableData;
  selectionData = [];
  userTableConfig: TableConfig;
  userValid$ = new Subject<boolean>();
  @ViewChild('userTable', { static: false }) userTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    public drawmodalservice: DrawModalService,
    public warningMessageService: WarningMessageService,
    private multiClustersServiceApi: ApiMultiClustersService
  ) {}

  ngOnInit() {
    this.initTableConfig();
    this.initTableData();
  }

  initTableData() {
    this.userTableData = {
      data: this.rowItem.authUserList,
      total: size(this.rowItem.authUserList)
    };
  }

  initTableConfig() {
    this.userTableConfig = {
      table: {
        async: false,
        columns: [
          {
            key: 'userName',
            name: this.i18n.get('system_local_cluster_username_label'),
            sort: true,
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'managedUserName',
            name: this.i18n.get('system_managed_cluster_username_label'),
            sort: true,
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          }
        ],
        compareWith: 'userId',
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        rows: this.rowItem.isView
          ? null
          : {
              selectionMode: 'multiple'
            },
        selectionChange: data => {
          this.selectionData = data;
          this.userValid$.next(!!size(data));
        }
      },
      pagination: {
        mode: 'simple',
        pageIndex: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      this.drawmodalservice.create(
        assign({}, MODAL_COMMON.drawerOptions, {
          lvType: 'modal',
          lvWidth: 580,
          lvHeight: 380,
          lvOkDisabled: true,
          lvModalKey: 'cancleAuthUserWarnModal',
          lvContent: CancleWarnComponent,
          lvAfterOpen: modal => {
            const content = modal.getContentComponent() as CancleWarnComponent;
            const modalIns = modal.getInstance();
            content.isChecked$.subscribe(res => {
              modalIns.lvOkDisabled = !res;
            });
          },
          lvComponentParams: {
            rowItem: { ...this.rowItem, cancelData: [...this.selectionData] }
          },
          lvOk: modal => {
            const promises = [];
            each(this.selectionData, item => {
              promises.push(
                new Promise((resolve, reject) => {
                  this.multiClustersServiceApi
                    .deleteManagedClustersAuth({
                      userId: item.userId,
                      clusterId: this.rowItem.clusterId
                    })
                    .subscribe(
                      res => {
                        resolve(res);
                      },
                      err => {
                        reject(err);
                      }
                    );
                })
              );
            });
            Promise.all(promises)
              .then(() => {
                observer.next();
                observer.complete();
              })
              .catch(err => {
                observer.error(err);
                observer.complete();
              });
          },
          lvCancel: () => {
            observer.error(null);
            observer.complete();
          }
        })
      );
    });
  }
}
