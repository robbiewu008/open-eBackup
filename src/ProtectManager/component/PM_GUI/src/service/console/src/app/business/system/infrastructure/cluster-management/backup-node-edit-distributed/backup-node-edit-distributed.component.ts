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
  Component,
  OnInit,
  Output,
  EventEmitter,
  ViewChild,
  ChangeDetectorRef,
  AfterViewInit
} from '@angular/core';
import { combineLatest, Observable, Observer } from 'rxjs';
import {
  CapacityCalculateLabel,
  CommonConsts,
  CAPACITY_UNIT,
  DataMapService,
  I18NService,
  WarningMessageService,
  LocalStorageApiService,
  LANGUAGE,
  BaseUtilService,
  MODAL_COMMON,
  StoragesApiService,
  CookieService,
  DataMap
} from 'app/shared';
import {
  BackupClustersApiService,
  ClustersApiService
} from 'app/shared/api/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  ProTableComponent,
  TableCols,
  TableConfig,
  TableData
} from 'app/shared/components/pro-table';
import { PacificNodeNetworkInfo } from 'app/shared/api/models/pacific-node-network-info';
import { AppUtilsService } from '../../../../../shared/services/app-utils.service';
import { size } from 'lodash';

@Component({
  selector: 'aui-backup-node-edit-distributed',
  templateUrl: './backup-node-edit-distributed.component.html',
  styleUrls: ['./backup-node-edit-distributed.component.less'],
  providers: [CapacityCalculateLabel]
})
export class BackupNodeEditDistributedComponent
  implements OnInit, AfterViewInit {
  activeIndex;
  drawData;
  ipLabel = this.i18n.get('system_management_ip_label');
  tableConfig: TableConfig;
  tableDataBackup: TableData;
  tableDataArchived: TableData;
  tableDataReplication: TableData;
  selectionData: PacificNodeNetworkInfo = {
    backupIpInfoList: [],
    archiveIpInfoList: [],
    replicationIpInfoList: []
  };
  selectionValid = new EventEmitter<any>();
  isDecouple = this.appUtilsService.isDecouple;

  @ViewChild('dataTableBackup', { static: false })
  dataTableBackup: ProTableComponent;
  @ViewChild('dataTableArchived', { static: false })
  dataTableArchived: ProTableComponent;
  @ViewChild('dataTableReplication', { static: false })
  dataTableReplication: ProTableComponent;

  constructor(
    public i18n: I18NService,
    public drawmodalservice: DrawModalService,
    public warningMessageService: WarningMessageService,
    public dataMapService: DataMapService,
    public clusterApiService: ClustersApiService,
    public baseUtilService: BaseUtilService,
    public virtualScroll: VirtualScrollService,
    public appUtilsService: AppUtilsService,
    public cdr?: ChangeDetectorRef
  ) {}

  ngOnInit(): void {
    this.initConfig();
    this.virtualScroll.getScrollParam(200);
  }

  ngAfterViewInit() {
    this.dataTableBackup.fetchData();
  }

  initConfig() {
    const cols: TableCols[] = [
      {
        key: 'ifaceName',
        name: this.isDecouple
          ? this.i18n.get('common_optional_port_label')
          : this.i18n.get('common_port_label')
      },
      {
        key: 'ipAddress',
        name: this.i18n.get('common_ip_address_mask_label')
      }
    ];

    this.tableConfig = {
      table: {
        compareWith: 'ipAddress',
        rows: {
          selectionMode: 'multiple',
          selectionTrigger: 'selector',
          showSelector: true
        },
        columns: cols,
        virtualScroll: true,
        scrollFixed: true,
        scroll: this.virtualScroll.scrollParam,
        colDisplayControl: false,
        selectionChange: selection => {
          if (this.activeIndex === 'backup') {
            this.selectionData.backupIpInfoList = selection;
          } else if (this.activeIndex === 'archived') {
            this.selectionData.archiveIpInfoList = selection;
          } else if (this.activeIndex === 'replication') {
            this.selectionData.replicationIpInfoList = selection;
          }
          if (this.appUtilsService.isDistributed) {
            this.selectionValid.emit(
              this.selectionData.backupIpInfoList.length === 0 ||
                this.selectionData.archiveIpInfoList.length === 0
            );
          } else {
            this.selectionValid.emit(
              this.selectionData.backupIpInfoList.length === 0
            );
          }
        },
        fetchData: () => {
          this.getData();
        }
      },
      pagination: {
        mode: 'simple',
        showPageSizeOptions: false,
        winTablePagination: true
      }
    };
  }

  getData() {
    this.clusterApiService
      .getPacificNodeDetail({ name: this.drawData.name })
      .subscribe(res => {
        this.tableDataBackup = {
          data: res.allNetworkInfo.backupIpInfoList,
          total: res.allNetworkInfo.backupIpInfoList.length
        };
        this.tableDataArchived = {
          data: res.allNetworkInfo.archiveIpInfoList,
          total: res.allNetworkInfo.archiveIpInfoList.length
        };
        if (this.isDecouple) {
          this.tableDataReplication = {
            data: res.allNetworkInfo.replicationIpInfoList,
            total: res.allNetworkInfo.replicationIpInfoList.length
          };
        }
        this.selectionData = res.usedNetworkInfo;
        this.dataTableBackup.setSelections(this.selectionData.backupIpInfoList);
        this.dataTableArchived?.setSelections(
          this.selectionData.archiveIpInfoList
        );
        if (this.isDecouple) {
          this.dataTableReplication?.setSelections(
            this.selectionData.replicationIpInfoList
          );
        }

        if (this.appUtilsService.isDistributed) {
          this.selectionValid.emit(
            size(this.dataTableBackup.selection) === 0 ||
              size(this.dataTableArchived.selection) === 0
          );
        } else {
          this.selectionValid.emit(size(this.dataTableBackup.selection) === 0);
        }
        this.cdr.detectChanges();
      });
  }

  selectIndexChange() {
    if (this.activeIndex === 'backup') {
      this.dataTableBackup.setSelections(this.selectionData.backupIpInfoList);
    } else if (this.activeIndex === 'archived') {
      this.dataTableArchived.setSelections(
        this.selectionData.archiveIpInfoList
      );
    } else if (this.activeIndex === 'replication') {
      this.dataTableReplication.setSelections(
        this.selectionData.replicationIpInfoList
      );
    }
  }

  onOK(): Observable<any> {
    return new Observable<any>((observer: Observer<any>) => {
      this.clusterApiService
        .updatePacificNodeNetworkInfo({
          name: this.drawData.name,
          request: this.selectionData
        })
        .subscribe(
          res => {
            observer.next(res);
            observer.complete();
          },
          error => {
            observer.error(error);
            observer.complete();
          }
        );
    });
  }
}
