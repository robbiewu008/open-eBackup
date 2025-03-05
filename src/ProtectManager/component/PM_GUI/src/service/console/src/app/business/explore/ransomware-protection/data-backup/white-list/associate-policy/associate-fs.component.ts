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
import { ModalRef } from '@databackup/live';
import {
  BackupCopyWhitelistManagementService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedResourceApiService,
  WarningMessageService
} from 'app/shared';
import { ProtectedResourcePageListResponse } from 'app/shared/api/models';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { assign, each, isArray, isEmpty, map, size, tail } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-data-backup-associate-fs',
  templateUrl: './associate-fs.component.html',
  styleUrls: ['./associate-fs.component.less']
})
export class AssociateFsComponent implements OnInit, AfterViewInit {
  rowData;
  isDisassociated: boolean;
  isDetail: boolean;

  tableConfig: TableConfig;
  tableData: TableData;
  selectionData;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('storageDeviceTpl', { static: true })
  storageDeviceTpl: TemplateRef<any>;

  constructor(
    private modal: ModalRef,
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private warningMessageService: WarningMessageService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private backupCopyWhitelistService: BackupCopyWhitelistManagementService
  ) {}

  ngAfterViewInit(): void {
    this.dataTable?.fetchData();
  }

  ngOnInit(): void {
    this.initTableConfig();
    this.getStorage();
  }

  getStorage() {
    if (!this.isDisassociated) {
      return;
    }
    this.appUtilsService
      .getCyberEngineStorage()
      .subscribe((res: ProtectedResourcePageListResponse) => {
        const deviceFilterMap = map(res.records, item => {
          return {
            key: item.uuid,
            label: item.name,
            value: item.name
          };
        });
        each(this.tableConfig.table.columns, item => {
          if (item.key === 'environment') {
            assign(item, {
              filter: {
                type: 'select',
                isMultiple: true,
                showCheckAll: true,
                options: deviceFilterMap
              }
            });
          }
        });
        this.dataTable.init();
      });
  }

  getOkDisabled() {
    this.modal.getInstance().lvOkDisabled = !size(this.selectionData);
  }

  initTableConfig() {
    this.tableConfig = {
      table: {
        compareWith: this.isDisassociated ? 'uuid' : 'fsId',
        columns: [
          {
            key: this.isDisassociated ? 'name' : 'fsName',
            name: this.i18n.get('protection_file_system_name_label'),
            filter: this.isDisassociated
              ? {
                  type: 'search',
                  filterMode: 'contains'
                }
              : null
          },
          {
            key: this.isDisassociated ? 'environment' : 'deviceName',
            name: this.i18n.get('protection_storage_device_label'),
            cellRender: this.isDisassociated ? this.storageDeviceTpl : null
          },
          {
            key: this.isDisassociated ? 'parentName' : 'vstoreName',
            name: this.i18n.get('common_tenant_label'),
            filter: this.isDisassociated
              ? {
                  type: 'search',
                  filterMode: 'contains'
                }
              : null
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

  getData(filters) {
    if (this.isDisassociated) {
      const params = {
        pageNo: filters.paginator.pageIndex,
        pageSize: filters.paginator.pageSize
      };
      const defaultConditions = {
        subType: [DataMap.Resource_Type.LocalFileSystem.value]
      };
      if (!isEmpty(filters.conditions_v2)) {
        const conditions = JSON.parse(filters.conditions_v2);
        if (conditions.environment) {
          assign(defaultConditions, {
            environment: {
              name: tail(conditions.environment)
            }
          });
          delete conditions.environment;
        }
        assign(defaultConditions, conditions);
      }
      assign(params, { conditions: JSON.stringify(defaultConditions) });
      this.protectedResourceApiService.ListResources(params).subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
    } else {
      const params = {
        pageNum: filters.paginator.pageIndex + 1,
        pageSize: filters.paginator.pageSize,
        id: isArray(this.rowData) ? map(this.rowData, 'id') : [this.rowData?.id]
      };
      this.backupCopyWhitelistService
        .getFsInfoByWhiteListItemId(params)
        .subscribe(res => {
          this.tableData = {
            data: res.records,
            total: res.totalCount
          };
        });
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const fsInfos = map(this.selectionData, item => {
        return {
          deviceId: item.environment?.uuid,
          deviceName: item.environment?.name,
          deviceType: item.environment?.subType,
          fsId: item.extendInfo?.fileSystemId,
          fsName: item.extendInfo?.fileSystemName,
          resourceId: item.uuid,
          vstoreId: item.extendInfo?.tenantId,
          vstoreName: item.extendInfo?.tenantName
        };
      });
      const whitelistIds = map(this.rowData, 'id');
      if (this.isDisassociated) {
        this.backupCopyWhitelistService
          .createWhitelistAssoc({
            createWhiteListAssociationReq: {
              ocWhiteListFilesystemInfos: fsInfos,
              whitelistIds
            }
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: () => {
              observer.error(null);
              observer.complete();
            }
          });
      } else {
        this.backupCopyWhitelistService
          .delWhitelistAssoc({
            delWhitelistAssocReq: {
              resourceIds: map(this.selectionData, 'resourceId'),
              whitelistIds
            }
          })
          .subscribe({
            next: () => {
              observer.next();
              observer.complete();
            },
            error: () => {
              observer.error(null);
              observer.complete();
            }
          });
      }
    });
  }
}
