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
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  CommonConsts,
  HostAgentUpdateControllerService,
  I18NService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { size, map, filter, pick } from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';

@Component({
  selector: 'aui-update-agent',
  templateUrl: './update-agent.component.html',
  styleUrls: ['./update-agent.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class UpdateAgentComponent implements OnInit, AfterViewInit {
  @Input() selection;
  checkboxStatus;
  tableData: TableData;
  tableConfig: TableConfig;
  isOpenStack = false;

  isChecked$ = new Subject<boolean>();

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('endpointTpl', { static: true })
  endpointTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private cdr: ChangeDetectorRef,
    private clientManagerApiService: HostAgentUpdateControllerService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
    let applicationArr = [];
    applicationArr = map(this.selection, item => {
      if (item.extendInfo?.agent_applications) {
        return JSON.parse(item.extendInfo?.agent_applications).menus;
      }
    });
    const arr = applicationArr.reduce((a, b) => a.concat(b));
    this.isOpenStack = arr?.some(item => {
      return (
        item.menuValue === 'Cloud Platform' &&
        item?.applications.some(
          v =>
            v.isChosen &&
            v.appValue ===
              'OpenStackContainer,OpenStackProject,OpenStackCloudServer'
        )
      );
    });
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'name',
        name: this.i18n.get('common_name_label')
      },
      {
        key: 'endpoint',
        name: this.i18n.get('common_ip_address_label'),
        cellRender: this.endpointTpl
      },
      {
        key: 'version',
        name: this.i18n.get('system_current_version_label')
      },
      {
        key: 'agentUpgradeableVersion',
        name: this.i18n.get('protection_upgradeable_version_label')
      }
    ];
    this.tableConfig = {
      table: {
        async: false,
        columns: cols,
        showLoading: false,
        colDisplayControl: false,
        size: 'small',
        fetchData: () => {
          this.getData();
        }
      },
      pagination: {
        pageIndex: CommonConsts.PAGE_START,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getData() {
    this.tableData = {
      data: map(this.selection, item => {
        item['agentUpgradeableVersion'] =
          item.extendInfo?.agentUpgradeableVersion;
        return item;
      }),
      total: size(this.selection)
    };
    this.cdr.detectChanges();
  }

  warningConfirmChange(e) {
    this.isChecked$.next(e);
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const agentClientInfoList = map(this.selection, item => {
        item['agentId'] = item.extendInfo?.agentId;
        return pick(item, 'uuid', 'agentId');
      });
      this.clientManagerApiService
        .batchUpdateAgentClientUsingPOST({
          request: { agentClientInfoList }
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
    });
  }
}
