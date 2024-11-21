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
  CookieService,
  ExternalSystemService,
  getPermissionMenuItem,
  I18NService,
  MODAL_COMMON,
  OperateItems,
  RoleType
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
import { each, filter as _filter, size, toString } from 'lodash';
import { CreateExternalSystemComponent } from './create-external-system/create-external-system.component';

@Component({
  selector: 'aui-external-associated-systems',
  templateUrl: './external-associated-systems.component.html',
  styleUrls: ['./external-associated-systems.component.less']
})
export class ExternalAssociatedSystemsComponent
  implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  itemOptsConfig;
  isSysAdmin = this.cookieService.role === RoleType.SysAdmin;
  tableIsHidden = true;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('nameTpl', { static: true }) nameTpl: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public externalSystemService: ExternalSystemService,
    public cdr?: ChangeDetectorRef,
    public virtualScroll?: VirtualScrollService,
    public cookieService?: CookieService
  ) {}

  ngOnInit(): void {
    this.initConfig();
  }

  ngAfterViewInit() {
    this.dataTable.fetchData();
  }

  initConfig() {
    const opts: ProButton[] = [
      {
        id: 'create',
        type: 'primary',
        permission: OperateItems.AddExternalAssociatedSystem,
        label: this.i18n.get('common_add_label'),
        onClick: () => {
          this.create();
        }
      }
    ];
    this.optsConfig = getPermissionMenuItem(opts);

    const itemOpts: ProButton[] = [
      {
        id: 'jump',
        label: this.i18n.get('common_goto_external_system_label'),
        permission: OperateItems.JumpExternalAssociatedSystem,
        onClick: data => this.jump(data[0])
      },
      {
        id: 'edit',
        label: this.i18n.get('common_edits_label'),
        permission: OperateItems.EditExternalAssociatedSystem,
        onClick: data => this.create(data[0])
      },
      {
        id: 'delete',
        label: this.i18n.get('common_delete_label'),
        permission: OperateItems.DeleteExternalAssociatedSystem,
        onClick: data =>
          this.externalSystemService
            .DeleteExternalSystem({ uuid: data[0].uuid })
            .subscribe(res => this.dataTable.fetchData())
      }
    ];
    this.itemOptsConfig = getPermissionMenuItem(itemOpts);

    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        },
        cellRender: this.isSysAdmin ? this.nameTpl : null
      },
      {
        key: 'type',
        name: this.i18n.get('common_backup_software_label')
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_label')
      },
      {
        key: 'uuid',
        name: this.i18n.get('common_uuid_label')
      },
      {
        key: 'port',
        name: this.i18n.get('common_port_label')
      },
      {
        key: 'username',
        name: this.i18n.get('common_username_label')
      },
      {
        key: 'operation',
        name: this.i18n.get('common_operation_label'),
        width: 240,
        useOpWidth: true,
        cellRender: {
          type: 'operation',
          config: {
            maxDisplayItems: 4,
            items: this.itemOptsConfig
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

  jump(data) {
    if (data.type === 'dpa') {
      this.jumpToDPA(data);
    } else {
      this.jumpToEBackup(data);
    }
  }

  jumpToDPA(data) {
    // DPA跳转用ip+端口跳转
    const url = `https://${encodeURI(data.endpoint)}:${encodeURI(
      toString(data.port)
    )}`;
    window.open(url, '_blank');
  }

  jumpToEBackup(data) {
    this.externalSystemService
      .GenerateExternalSystemToken({ uuid: data.uuid })
      .subscribe(res => {
        const language = this.i18n.isEn ? 'en' : 'zh';
        const url = `https://${encodeURI(res.ip)}:${encodeURI(
          toString(res.port)
        )}/eBackup/#/login?token=${encodeURIComponent(
          res.token
        )}&language=${encodeURIComponent(language)}`;
        window.open(url, '_blank');
      });
  }

  create(data?) {
    this.drawModalService.create({
      ...MODAL_COMMON.generateDrawerOptions(),
      lvHeader: data
        ? this.i18n.get('common_modify_label')
        : this.i18n.get('common_add_label'),
      lvModalKey: 'create_external_system',
      lvWidth: MODAL_COMMON.normalWidth,
      lvContent: CreateExternalSystemComponent,
      lvOkDisabled: true,
      lvAfterOpen: modal => {
        const content = modal.getContentComponent() as CreateExternalSystemComponent;
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
          const content = modal.getContentComponent() as CreateExternalSystemComponent;
          content.onOK().subscribe({
            next: res => {
              resolve(true);
              this.dataTable.fetchData();
            },
            error: () => resolve(false)
          });
        });
      }
    });
  }

  getData(filters?: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };

    each(filters.filters, filter => {
      if (filter.value && size(filter.value)) {
        if (filter.filterMode === 'contains') {
          params[filter.key] = filter.value[0];
        } else {
          params[filter.key] = filter.value;
        }
      }
    });

    this.externalSystemService.ShowExternalSystemInfo(params).subscribe(res => {
      this.tableData = {
        data: res.records,
        total: res.totalCount
      };
      this.tableIsHidden = !filters.filters.length && !res.totalCount;
      this.cdr.detectChanges();
    });
  }
}
