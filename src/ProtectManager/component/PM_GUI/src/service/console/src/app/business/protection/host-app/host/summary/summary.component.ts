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
import { Component, Input, OnInit } from '@angular/core';
import {
  CommonConsts,
  CookieService,
  DataMap,
  DataMapService,
  I18NService,
  filterApplication
} from 'app/shared';
import {
  AgentLanFreeAixControllerService,
  AgentLanFreeControllerService,
  ClientManagerApiService,
  CopiesService,
  ProtectedResourceApiService
} from 'app/shared/api/services';
import { NumberToFixed } from 'app/shared/components/pro-core';
import { TableConfig, TableData } from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { each, find, get, isNumber, map, size } from 'lodash';

@Component({
  selector: 'aui-host-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private dataMapService: DataMapService,
    public copiesApiService: CopiesService,
    private agentLanFreeService: AgentLanFreeControllerService,
    private agentLanFreeAixService: AgentLanFreeAixControllerService,
    private clientManagerApiService: ClientManagerApiService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private appUtilsService: AppUtilsService
  ) {}

  @Input() source;
  NumberToFixed = NumberToFixed;
  size = size;

  volumeTree = [];
  sourceType = DataMap.Resource_Type.ABBackupClient.value;
  showProtectedInfos = false;
  wwpnTableConfig: TableConfig;
  wwpnTableData: TableData;
  fcPortTableConfig: TableConfig;
  fcPortTableData: TableData;
  iqnTableData;
  dataProtocol;
  sanclientTableConfig: TableConfig;
  sanclientTableData: TableData;
  isX3000 = this.i18n.get('deploy_type') === DataMap.Deploy_Type.x3000.value;
  data;
  sanclientResourseIds = [];
  dataMap = DataMap;
  isLanfree = false;
  isSanclient = false;
  isSanclientFC = false;
  isSanclientIqn = false;
  isAix = false;
  resourceData = [];
  applicationList = [];
  node;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  isHcsEnvir =
    this.cookieService.get('serviceProduct') === CommonConsts.serviceProduct;

  initDetailData(data: any) {
    this.source = data;
    if (data.subType === DataMap.Resource_Type.UBackupAgent.value) {
      this.clientManagerApiService
        .querySupportApplicationGET({
          lang: this.i18n.language,
          uuid: data.uuid
        })
        .subscribe(res => {
          each(res, item => {
            this.applicationList.push({
              ...item,
              label: item.menuDesc,
              isLeaf: false,
              children: map(
                filterApplication(item, this.appUtilsService),
                v => {
                  return {
                    ...v,
                    label: v.appDesc,
                    isLeaf: true
                  };
                }
              )
            });
          });
          this.resourceData = this.applicationList;
        });
    }
  }

  ngOnInit() {
    this.isLanfree = this.source?.extendInfo?.isAddLanFree === '1';
    this.isSanclient =
      this.isLanfree &&
      this.source.subType === DataMap.Resource_Type.SBackupAgent.value;
    this.isAix =
      this.isLanfree && this.source.osType === DataMap.Os_Type.aix.value;
    if (this.isHcsUser) {
      this.isLanfree = false;
    }
    this.initTable();
    if (!this.isAix && this.isLanfree) {
      this.getNode();
    } else {
      this.getLanFree();
    }
  }

  getNode() {
    this.agentLanFreeService
      .queryClusterNodeInfoGET({ agentId: this.source?.uuid })
      .subscribe(res => {
        this.node = find(res, { chosen: true });
        this.getLanFree(this.node);
      });
  }

  initTable() {
    this.wwpnTableConfig = {
      table: {
        columns: [
          { key: 'wwpn', name: 'WWPN' },
          {
            key: 'runningStatus',
            name: this.i18n.get('protection_running_status_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('lanFreeRunningStatus')
            }
          }
        ],
        size: 'small',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true
      }
    };
    this.fcPortTableConfig = {
      table: {
        columns: [
          {
            key: 'location',
            name: this.i18n.get('common_location_label'),
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'runningStatus',
            name: this.i18n.get('protection_running_status_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('lanFreeRunningStatus')
            }
          },
          {
            key: 'wwpn',
            name: 'WWPN',
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          }
        ],
        size: 'small',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true
      }
    };
    this.sanclientTableConfig = {
      table: {
        columns: [
          {
            key: 'wwpn',
            name: 'WWPN',
            hidden: !this.isSanclient
          },
          {
            key: 'runningStatus',
            name: this.i18n.get('protection_running_status_label'),
            hidden: !this.isSanclient,
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('lanFreeRunningStatus')
            }
          },
          {
            key: 'name',
            name: this.i18n.get('common_name_label'),
            hidden: !this.isAix
          },
          {
            key: 'endpoint',
            name: this.i18n.get('common_ip_address_label'),
            hidden: !this.isAix
          }
        ],
        fake: true,
        size: 'small',
        colDisplayControl: false
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        pageSize: CommonConsts.PAGE_SIZE_SMALL,
        winTablePagination: true
      }
    };
  }

  getSanclientDetail(recordsTemp?, startPage?) {
    const params = {
      pageNo: startPage || CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      conditions: JSON.stringify({
        type: 'Host',
        subType: [DataMap.Resource_Type.SBackupAgent.value],
        isCluster: [['=='], false]
      })
    };
    this.protectedResourceApiService.ListResources(params).subscribe(res => {
      if (!recordsTemp) {
        recordsTemp = [];
      }
      if (!isNumber(startPage)) {
        startPage = CommonConsts.PAGE_START;
      }
      startPage++;
      recordsTemp = [...recordsTemp, ...res.records];
      if (
        startPage === Math.ceil(res.totalCount / CommonConsts.PAGE_SIZE) ||
        res.totalCount === 0
      ) {
        const hostArray = [];
        each(recordsTemp, item => {
          each(this.sanclientResourseIds, tmp => {
            if (item.uuid === tmp.sanclientResourceIds) {
              hostArray.push({
                ...item
              });
            }
          });
        });
        this.sanclientTableData = {
          data: hostArray,
          total: size(hostArray)
        };
        return;
      }
      this.getSanclientDetail(recordsTemp, startPage);
    });
  }

  getNormalData(res) {
    this.wwpnTableData = {
      data: res.wwpns,
      total: size(res.wwpns)
    };
    this.fcPortTableData = {
      data: res.fcPorts,
      total: size(res.fcPorts)
    };
  }

  getLanFree(node?) {
    if (!this.isLanfree) {
      return;
    }
    if (this.isAix) {
      this.agentLanFreeAixService
        .queryLanFreeAixUsingGET({
          agentId: this.source.uuid
        })
        .subscribe(res => {
          each(res.sanclientResourceIds, item => {
            this.sanclientResourseIds.push({
              sanclientResourceIds: item
            });
          });
          this.dataProtocol = res.dataProtocol;
          if (res.clientIqns) {
            this.iqnTableData = res.clientIqns[0];
          }
          if (res.clientWwpns) {
            this.wwpnTableData = {
              data: res.clientWwpns,
              total: size(res.clientWwpns)
            };
          }
          this.data = res;
          this.getSanclientDetail();
        });
    } else {
      this.agentLanFreeService
        .queryLanFreeGET({ agentId: this.source.uuid, memberEsn: node.esn })
        .subscribe((res: any) => {
          if (this.isSanclient) {
            this.sanclientTableData = {
              data: res.sanClientWwpns,
              total: size(res.sanClientWwpns)
            };
            if (!!res.sanClientIqns.length) {
              this.isSanclientIqn = true;
              let iqnData = [];
              each(res.sanClientIqns, item => {
                iqnData.push({
                  clientIqn: item
                });
              });
              this.iqnTableData = {
                data: iqnData,
                total: size(iqnData)
              };
            }
            if (!!res.wwpns.length) {
              this.isSanclientFC = true;
              this.getNormalData(res);
            }
          } else {
            this.getNormalData(res);
          }
        });
    }
  }
}
