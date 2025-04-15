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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  CookieService,
  getPermissionMenuItem,
  I18NService,
  KerberosAPIService,
  MODAL_COMMON,
  OperateItems,
  WarningMessageService
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  each,
  filter as _filter,
  first,
  isFunction,
  map,
  reject,
  size,
  toString
} from 'lodash';
import { CreateKerberosComponent } from './create-kerberos/create-kerberos.component';

@Component({
  selector: 'aui-kerberos',
  templateUrl: './kerberos.component.html',
  styleUrls: ['./kerberos.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class KerberosComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  isHcsUser = false;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public kerberosApi?: KerberosAPIService,
    public cdr?: ChangeDetectorRef,
    public virtualScroll?: VirtualScrollService,
    public warningMessageService?: WarningMessageService
  ) {}

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.getHcsUser();
    this.initConfig();
    this.virtualScroll.getScrollParam(200);
  }

  getHcsUser() {
    const cookies = document.cookie.split(';');
    for (let i = 0; i < cookies.length; i++) {
      const cookiePair = cookies[i].split('=');
      if (cookiePair[0].trim() === 'userType') {
        this.isHcsUser =
          decodeURIComponent(cookiePair[1]) === CommonConsts.HCS_USER_TYPE;
      }
    }
  }

  gotoPreviousRoute() {
    window.history.back();
  }

  onChange() {
    this.ngOnInit();
    this.ngAfterViewInit();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'create',
        type: 'primary',
        permission: OperateItems.CreateKerberos,
        label: this.i18n.get('common_add_label'),
        onClick: () => {
          this.create();
        }
      },
      {
        id: 'modify',
        permission: OperateItems.ModifyKerberos,
        label: this.i18n.get('common_modify_label'),
        onClick: data => {
          this.create(first(data));
        }
      },
      {
        id: 'delete',
        permission: OperateItems.DeleteKerberos,
        label: this.i18n.get('common_delete_label'),
        onClick: data => {
          this.delete(data);
        },
        disableCheck: data => {
          return !size(data);
        }
      }
    ];
    const optItems = getPermissionMenuItem(opts);
    this.optsConfig = _filter(optItems, { id: 'create' });

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'principalName',
        name: this.i18n.get('common_principal_name_label')
      },
      {
        key: 'operation',
        width: 130,
        name: this.i18n.get('common_operation_label'),
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 1,
            items: reject(optItems, { id: 'create' })
          }
        }
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'uuid',
        columns: cols,
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        }
      }
    };
  }

  getData(filters?: Filters) {
    const params = {
      startPage: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    each(filters.filters, filter => {
      if (filter.value && size(filter.value)) {
        params[filter.key] = filter.value;
      }
    });

    this.kerberosApi.queryAllKerberosUsingGET(params).subscribe(res => {
      this.tableData = {
        data: res.items,
        total: res.total
      };
      this.cdr.detectChanges();
    });
  }

  create(data?, callback?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_add_label'),
      lvModalKey: 'create_kerberos',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: CreateKerberosComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateKerberosComponent;
        const modalIns = modal.getInstance();
        content.formGroup.statusChanges.subscribe(res => {
          modalIns.lvOkDisabled = res !== 'VALID';
        });
      },
      lvComponentParams: {
        data
      },
      lvOk: modal => {
        return new Promise(resolve => {
          const content = modal.getContentComponent() as CreateKerberosComponent;
          content.onOK().subscribe({
            next: res => {
              resolve(true);
              if (callback && isFunction(callback)) {
                callback(res);
              } else {
                this.dataTable.fetchData();
              }
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  delete(data) {
    this.warningMessageService.create({
      content: this.i18n.get('system_delete_kerberos_label'),
      onOK: () => {
        const kerberosId = toString(map(data, 'kerberosId'));
        this.kerberosApi
          .deleteKerberosUsingDELETE({ kerberosId })
          .subscribe(res => {
            this.dataTable.fetchData();
          });
      }
    });
  }
}
