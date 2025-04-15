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
import { AfterViewInit, Component, OnInit, ViewChild } from '@angular/core';
import {
  DataMap,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  OpHcsServiceApiService,
  OperateItems,
  WarningMessageService,
  getPermissionMenuItem
} from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  assign,
  filter,
  first,
  includes,
  isArray,
  isEmpty,
  values
} from 'lodash';
import { StorResourceNodeComponent } from 'app/business/protection/cloud/huawei-stack/register-huawei-stack/store-resource-node/store-resource-node.component';

@Component({
  selector: 'aui-hcs-storage',
  templateUrl: './hcs-storage.component.html',
  styleUrls: ['./hcs-storage.component.less']
})
export class HcsStorageComponent implements OnInit, AfterViewInit {
  tableConfig: TableConfig;
  tableData: TableData;
  optsConfig;
  selectionData = [];

  scaleoutLabel = this.i18n.get('protection_database_type_block_storage_label');
  centralizedLabel = this.i18n.get('common_san_storage_label');

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private drawModalService: DrawModalService,
    private warningMessageService: WarningMessageService,
    private opHcsServiceApiService: OpHcsServiceApiService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable.fetchData();
  }

  ngOnInit() {
    this.initConfig();
  }

  initConfig() {
    const opts: { [key: string]: ProButton } = {
      add: {
        id: 'add',
        type: 'primary',
        permission: OperateItems.AddHcsStorage,
        label: this.i18n.get('common_add_label'),
        onClick: () => this.add()
      },
      modify: {
        id: 'modify',
        permission: OperateItems.AddHcsStorage,
        label: this.i18n.get('common_modify_label'),
        onClick: ([data]) => this.add(data)
      },
      delete: {
        id: 'delete',
        permission: OperateItems.AddHcsStorage,
        label: this.i18n.get('common_delete_label'),
        onClick: ([data]) => this.delete(data)
      }
    };
    this.optsConfig = getPermissionMenuItem([opts.add]);
    this.tableConfig = {
      table: {
        compareWith: 'ipList',
        columns: [
          {
            key: 'storageType',
            name: this.i18n.get('common_type_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('hcsStorageType')
            }
          },
          {
            key: 'ipList',
            name: this.i18n.get('common_management_ip_label')
          },
          {
            key: 'port',
            name: this.i18n.get('common_port_label')
          },
          {
            key: 'operation',
            width: 130,
            hidden: 'ignoring',
            name: this.i18n.get('common_operation_label'),
            cellRender: {
              type: 'operation',
              config: {
                maxDisplayItems: 1,
                items: getPermissionMenuItem(
                  filter(values(opts), item =>
                    includes(['modify', 'delete'], item.id)
                  )
                )
              }
            }
          }
        ],
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getData(filter);
        },
        trackByFn: (_, item) => {
          return item.ipList;
        }
      }
    };
  }

  getData(filters: Filters) {
    const params = {
      pageNo: filters.paginator.pageIndex,
      pageSize: filters.paginator.pageSize
    };
    this.opHcsServiceApiService
      .getAllStorageForHcs(params)
      .subscribe((res: any) => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  modifyStorage(res, resolve, modifyData) {
    const params = {
      storageType: `${res.storageType}`,
      ip: <string>first(res.ip),
      ipList: isArray(res.ip) ? res.ip.join(',') : res.ip,
      port: res.port,
      username: res.username,
      password: res.password,
      enableCert: res.enableCert,
      certification: res.certification,
      revocationList: res.revocationList,
      certName: res.certName,
      certSize: res.certSize,
      crlName: res.crlName,
      crlSize: res.crlSize
    };
    if (Number(res.storageType) === 1) {
      assign(params, {
        vbsNodeInfo: res.isVbsNodeInfo,
        vbsNodeUserName: res.vbsNodeUserName,
        vbsNodeIp: res.vbsNodeIp,
        vbsNodePort: res.vbsNodePort,
        vbsNodePassword: res.vbsNodePassword
      });
    }
    if (isEmpty(modifyData)) {
      this.opHcsServiceApiService
        .initStorageResources({ storageList: [params] })
        .subscribe({
          next: () => {
            resolve(true);
            this.dataTable?.fetchData();
          },
          error: () => resolve(false)
        });
    } else {
      assign(params, { uuid: modifyData.uuid });
      this.opHcsServiceApiService
        .editStorageForHcs({ storage: params })
        .subscribe({
          next: () => {
            resolve(true);
            this.dataTable?.fetchData();
          },
          error: () => resolve(false)
        });
    }
  }

  add(data?) {
    this.drawModalService.create(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'add-hcs-storage',
        lvWidth: MODAL_COMMON.normalWidth + 150,
        lvHeader: isEmpty(data)
          ? this.i18n.get('common_add_label')
          : this.i18n.get('common_modify_label'),
        lvContent: StorResourceNodeComponent,
        lvOkDisabled: true,
        lvComponentParams: {
          data: data
            ? {
                ip: data.ipList || data.ip,
                port: data.port,
                username: data.username,
                enableCert: data.enableCert,
                certification: data.certification,
                certName: data.certName,
                certSize: data.certSize,
                revocationList: data.revocationList,
                crlName: data.crlName,
                crlSize: data.crlSize,
                storageType:
                  data.storageType === '1'
                    ? this.scaleoutLabel
                    : this.centralizedLabel,
                isVbsNodeInfo: data.vbsNodeInfo,
                vbsNodeUserName: data.vbsNodeUserName,
                vbsNodeIp: data.vbsNodeIp,
                vbsNodePort: data.vbsNodePort
              }
            : null,
          subType: DataMap.Resource_Type.HCS.value,
          isModifyHcsStorage: !isEmpty(data)
        },
        lvOk: modal => {
          return new Promise(resolve => {
            const content = modal.getContentComponent() as StorResourceNodeComponent;
            content
              .onOK()
              .subscribe(res => this.modifyStorage(res, resolve, data));
          });
        }
      })
    );
  }

  delete(data) {
    if (isEmpty(data)) {
      return;
    }
    this.warningMessageService.create({
      content: this.i18n.get('protection_dorado_system_delete_label', [
        data.ipList
      ]),
      onOK: () => {
        this.opHcsServiceApiService
          .deleteStorageForHcs({ uuid: data.uuid })
          .subscribe(() => this.dataTable?.fetchData());
      }
    });
  }
}
