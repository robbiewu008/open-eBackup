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
import {
  CAPACITY_UNIT,
  CommonConsts,
  DataMapService,
  I18NService,
  MODAL_COMMON,
  SoftwareType,
  isNotFileSystem
} from 'app/shared';
import { BackupCopyDetectService } from 'app/shared/api/services/backup-copy-detect.service';
import {
  Filters,
  ProTableComponent,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { assign, includes, isEmpty } from 'lodash';
import { AssociatedSnapshotComponent } from '../associated-snapshot/associated-snapshot.component';
import { AppUtilsService } from 'app/shared/services/app-utils.service';

@Component({
  selector: 'aui-po-report-detail',
  templateUrl: './po-report-detail.component.html',
  styleUrls: ['./po-report-detail.component.less']
})
export class PoReportDetailComponent implements OnInit, AfterViewInit {
  rowData;
  snapshotId;

  tableConfig: TableConfig;
  tableData: TableData;
  capacityData = [];
  unitconst = CAPACITY_UNIT;
  softwareType = SoftwareType;

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('dateTpl', { static: true }) dateTpl: TemplateRef<any>;
  @ViewChild('copySizeTpl', { static: true }) copySizeTpl: TemplateRef<any>;
  @ViewChild('backupTaskExtraTpl', { static: true })
  backupTaskExtraTpl: TemplateRef<any>;
  @ViewChild('copyTypeTpl', { static: true }) copyTypeTpl: TemplateRef<any>;
  @ViewChild('fileCountExtraTpl', { static: true })
  fileCountExtraTpl: TemplateRef<any>;
  @ViewChild('infectedFileCountExtraTpl', { static: true })
  infectedFileCountExtraTpl: TemplateRef<any>;
  @ViewChild('fileCountTpl', { static: true })
  fileCountTpl: TemplateRef<any>;
  @ViewChild('infectFileCountTpl', { static: true })
  infectFileCountTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private dataMapService: DataMapService,
    private appUtilsService: AppUtilsService,
    private drawModalService: DrawModalService,
    private backupCopyDetectService: BackupCopyDetectService
  ) {}

  ngAfterViewInit() {
    if (this.dataTable) {
      this.dataTable.fetchData();
    }
  }

  ngOnInit() {
    this.initTableConfig();
  }

  initTableConfig() {
    this.tableConfig = {
      table: {
        compareWith: 'id',
        columns: [
          {
            key: 'backupTime',
            name: this.i18n.get('explore_copy_generation_time_label'),
            width: 160,
            cellRender: this.dateTpl
          },
          {
            key: 'backupType',
            name: this.i18n.get('common_backup_type_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('CopyData_Backup_Type')
            }
          },
          {
            key: 'backupJobId',
            thExtra: this.backupTaskExtraTpl,
            name: this.i18n.get('explore_backup_task_label'),
            width: this.i18n.isEn ? 170 : 140,
            filter: {
              type: 'search',
              filterMode: 'contains'
            }
          },
          {
            key: 'copySize',
            name: this.i18n.get('explore_copy_size_label'),
            cellRender: this.copySizeTpl,
            hidden: includes([SoftwareType.VEEAM], this.rowData.software)
          },
          {
            key: 'copyPath',
            name: this.i18n.get('common_file_path_label')
          },
          {
            key: 'status',
            name: this.i18n.get('explore_safe_status_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('detectionSnapshotStatus')
            },
            filter: {
              type: 'select',
              isMultiple: true,
              showCheckAll: true,
              options: this.dataMapService.toArray('detectionSnapshotStatus')
            }
          },
          {
            key: 'varyType',
            name: this.i18n.get('explore_vary_type_label'),
            cellRender: {
              type: 'status',
              config: this.dataMapService.toArray('snapshotVaryType')
            }
          },
          {
            key: 'copyType',
            name: this.i18n.get('explore_backup_application_type_label'),
            cellRender: this.copyTypeTpl
          },
          {
            key: 'fileCount',
            name: this.i18n.get('explore_associated_file_count_label'),
            thExtra: this.fileCountExtraTpl,
            cellRender: this.fileCountTpl,
            hidden: includes([SoftwareType.VEEAM], this.rowData.software)
          },
          {
            key: 'abnormalFileCount',
            name: this.i18n.get('explore_infected_file_count_label'),
            thExtra: this.infectedFileCountExtraTpl,
            cellRender: this.infectFileCountTpl,
            hidden: includes([SoftwareType.VEEAM], this.rowData.software)
          }
        ],
        scrollFixed: true,
        colDisplayControl: false,
        fetchData: (filter: Filters) => {
          this.getSnapshot(filter);
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

  getAppType(item) {
    return item.copyType & 0xff;
  }

  isVMware(item) {
    return isNotFileSystem(this.getAppType(item));
  }

  getCopyType(item): string {
    return this.appUtilsService.getCopyType(
      this.getAppType(item),
      this.rowData.software
    );
  }

  getSnapshot(filters: Filters) {
    const params = {
      pageNum: filters.paginator.pageIndex + 1,
      pageSize: filters.paginator.pageSize,
      fileSystemId: this.rowData?.fileSystemId,
      protectedObjName: this.rowData?.protectedObjName,
      snapshotId: this.snapshotId
    };

    if (!isEmpty(filters.conditions)) {
      const conditions = JSON.parse(filters.conditions);
      if (!isEmpty(conditions.status)) {
        assign(params, {
          statusList: conditions.status
        });
      }
      if (!isEmpty(conditions.backupJobId)) {
        assign(params, {
          backupJobId: conditions.backupJobId
        });
      }
    }

    this.backupCopyDetectService
      .QueryBackupsByProtectedObjName(params)
      .subscribe(res => {
        this.tableData = {
          data: res.records,
          total: res.totalCount
        };
      });
  }

  fileDetail(rowData) {
    if (
      includes([SoftwareType.VEEAM], this.rowData?.software) ||
      this.isVMware(rowData)
    ) {
      return;
    }
    this.drawModalService.openDetailModal(
      assign({}, MODAL_COMMON.generateDrawerOptions(), {
        lvModalKey: 'snapshot-file-detail',
        lvHeader: rowData?.name,
        lvWidth: MODAL_COMMON.xLargeWidth,
        lvContent: AssociatedSnapshotComponent,
        lvComponentParams: {
          rowData,
          software: this.rowData.software,
          snapshotId: this.snapshotId
        },
        lvFooter: [
          {
            label: this.i18n.get('common_close_label'),
            onClick: modal => modal.close()
          }
        ]
      })
    );
  }
}
