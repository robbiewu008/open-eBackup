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
import { FormBuilder, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  DataMap,
  DataMapService,
  I18NService,
  quaDrantTable,
  SystemApiService
} from 'app/shared';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { assign, countBy, defer, find, get, includes, nth, some } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-reuse-port',
  templateUrl: './reuse-port.component.html',
  styleUrls: ['./reuse-port.component.less']
})
export class ReusePortComponent implements OnInit {
  data;
  memberEsn;
  controlType;
  portData;
  formGroup: FormGroup;
  tableConfig: TableConfig;
  tableData: TableData;
  dataMap = DataMap;

  selectionData = [];
  isX9000 = includes(
    [DataMap.Deploy_Type.x9000.value],
    this.i18n.get('deploy_type')
  );
  disableRoleRadio = {
    backup: false,
    replicate: false,
    archive: false
  };

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;

  constructor(
    public modal: ModalRef,
    private fb: FormBuilder,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private systemApiService: SystemApiService
  ) {}

  ngOnInit(): void {
    this.portData = this.portData
      .map(item =>
        assign(item, {
          ip: item.IPV4ADDR || item.IPV6ADDR
        })
      )
      .filter(item => item.HOMECONTROLLERID === this.controlType);
    this.initForm();
    this.initTable();
  }

  initForm() {
    this.formGroup = this.fb.group({
      role: [DataMap.initRole.data.value]
    });

    this.formGroup.get('role').valueChanges.subscribe(res => {
      this.selectionData = [];
      this.dataTable.setSelections([]);
      this.modal.getInstance().lvOkDisabled = !this.selectionData.length;
      let tmpData = this.portData.filter(item => item.ROLE === res);
      if (res === DataMap.initRole.data.value) {
        // 备份类型的逻辑端口，复用需要是nfs+cifs以及可以漂移
        tmpData = tmpData.filter(
          item =>
            item?.SUPPORTPROTOCOL ===
              DataMap.initSupportPortocol.nfsCifs.value && !!item?.CANFAILOVER
        );
      }
      this.tableData = {
        data: tmpData,
        total: tmpData.length
      };
    });
  }

  getControlType(item) {
    // x9000根据象限划分控制器
    let Quadrant = nth(item.location.split('.'), -2);
    if (!this.controlType.includes(item.ownIngController)) {
      return false;
    }
    return some(['A', 'B', 'C', 'D'], type => {
      return (
        this.controlType.includes(type) &&
        includes(quaDrantTable[type], Quadrant)
      );
    });
  }

  initRoles() {
    const currentControllerRoleData = countBy(
      this.data.logicPortDtoList.filter(
        item => item.homeControllerId === this.controlType
      ),
      'role'
    );
    this.disableRoleRadio.backup =
      get(currentControllerRoleData, DataMap.initRole.data.value, 0) >= 8;
    this.disableRoleRadio.replicate =
      get(currentControllerRoleData, DataMap.initRole.copy.value, 0) >= 8;
    this.disableRoleRadio.archive =
      get(currentControllerRoleData, DataMap.initRole.dataManage.value, 0) >= 4;
    this.formGroup
      .get('role')
      .setValue(
        !this.disableRoleRadio.backup
          ? DataMap.initRole.data.value
          : !this.disableRoleRadio.replicate
          ? DataMap.initRole.copy.value
          : DataMap.initRole.dataManage.value
      );
  }

  initTable() {
    const cols: TableCols[] = [
      {
        key: 'NAME',
        name: this.i18n.get('common_name_label'),
        filter: {
          type: 'search',
          filterMode: 'contains'
        }
      },
      {
        key: 'ROLE',
        name: this.i18n.get('common_role_label'),
        cellRender: {
          type: 'status',
          config: this.dataMapService.toArray('initRoleTable')
        }
      },
      {
        key: 'ip',
        name: this.i18n.get('common_ip_address_label')
      }
    ];

    this.tableConfig = {
      table: {
        columns: cols,
        async: false,
        compareWith: 'NAME',
        rows: {
          selectionMode: 'single',
          selectionTrigger: 'selector',
          showSelector: true
        },
        colDisplayControl: false,
        selectionChange: selection => {
          this.selectionData = selection;
          this.modal.getInstance().lvOkDisabled = !this.selectionData.length;
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: true,
        winTablePagination: true
      }
    };
    const tmpData = this.portData.filter(
      item => item.ROLE === DataMap.initRole.data.value
    );
    this.tableData = {
      data: tmpData,
      total: tmpData.length
    };
    defer(() => this.initRoles());
  }

  getParams(item) {
    const params: any = {
      id: item.ID,
      name: item.NAME,
      homePortName: item.HOMEPORTNAME,
      homePortType: item.HOMEPORTTYPE,
      homeportId: item.HOMEPORTID,
      ip: item.IPV4ADDR || item.IPV6ADDR,
      mask: item.IPV4MASK || item.IPV6MASK,
      gateWay: item.IPV4GATEWAY || item.IPV6GATEWAY,
      role: item.ROLE,
      ipType: !!item.IPV4ADDR ? 'IPV4' : 'IPV6',
      supportProtocol: item.SUPPORTPROTOCOL,
      homeControllerId: this.controlType,
      currentControllerId: this.controlType
    };
    if (item.HOMEPORTTYPE === DataMap.initHomePortType.vlan.value) {
      const portVlan = find(
        this.data.vlanList,
        vlan =>
          vlan?.bondPortId === item.HOMEPORTID || vlan.id === item.HOMEPORTID
      );
      params.vlan = portVlan;
      params.bondPort = {
        id: item.HOMEPORTID,
        portNameList: portVlan.portNameList,
        mtu: portVlan.mtu
      };
    }
    if (item.HOMEPORTTYPE === DataMap.initHomePortType.bonding.value) {
      const portBond = find(
        this.data.bondPortList,
        bond => bond.id === item.HOMEPORTID
      );
      params.bondPort = {
        id: item.HOMEPORTID,
        portNameList: portBond.bondInfo.split(','),
        mtu: portBond.mtu
      };
    }
    return params;
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams(this.selectionData[0]);
      this.systemApiService
        .addLogicPorts({
          model: params,
          memberEsn: this.memberEsn || ''
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
