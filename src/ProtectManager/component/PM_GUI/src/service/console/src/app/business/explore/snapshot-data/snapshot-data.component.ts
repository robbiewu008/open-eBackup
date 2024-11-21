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
import { DatePipe } from '@angular/common';
import { Component, OnInit, ViewChild } from '@angular/core';
import {
  CommonConsts,
  CopiesDetectReportService,
  CopiesService,
  DataMap,
  I18NService,
  SYSTEM_TIME,
  WarningMessageService
} from 'app/shared';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import { assign, cloneDeep, find, includes, isEmpty, map } from 'lodash';
import { ResourceListComponent } from './resource-list/resource-list.component';
import { SnapshotListComponent } from './snapshot-list/snapshot-list.component';

@Component({
  selector: 'aui-snapshot-data',
  templateUrl: './snapshot-data.component.html',
  styleUrls: ['./snapshot-data.component.less'],
  providers: [DatePipe]
})
export class SnapshotDataComponent implements OnInit {
  selectedViewType = '0';
  deleteBtnDisable = true;
  isCyberEngine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;
  copyDataTipLabel = this.isCyberEngine
    ? this.i18n.get('common_snapshot_label')
    : this.i18n.get('common_copies_label');

  totalResource = 0;
  totalInfected = 0;
  totalUnInfected = 0;
  totalDetecting = 0;
  totalSnapshot = 0;
  totalNodetecte = 0;
  isEn = this.i18n.isEn;

  @ViewChild(ResourceListComponent, { static: false })
  ResourceListComponent: ResourceListComponent;
  @ViewChild(SnapshotListComponent, { static: false })
  SnapshotListComponent: SnapshotListComponent;

  constructor(
    private i18n: I18NService,
    private datePipe: DatePipe,
    private copiesApiService: CopiesService,
    private batchOperateService: BatchOperateService,
    private warningMessageService: WarningMessageService,
    private copiesDetectReportService: CopiesDetectReportService
  ) {}

  ngOnInit(): void {
    this.getResource();
    this.getResource(DataMap.detectionSnapshotStatus.infected.value);
    this.getResource(DataMap.detectionSnapshotStatus.uninfected.value);
    this.getResource(DataMap.detectionSnapshotStatus.detecting.value);
    this.getResource(DataMap.detectionSnapshotStatus.nodetecte.value);
    this.getSnapshot();
  }

  getResource(status?, mask = true) {
    const params = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: mask,
      resourceSubType: DataMap.Resource_Type.LocalFileSystem.value
    };
    if (status) {
      assign(params, { conditions: JSON.stringify({ status: [status] }) });
    }
    this.copiesDetectReportService
      .ShowDetectionStatistics(params)
      .subscribe(res => {
        if (status) {
          if (status === DataMap.detectionSnapshotStatus.infected.value) {
            this.totalInfected = res.total;
          }
          if (status === DataMap.detectionSnapshotStatus.uninfected.value) {
            this.totalUnInfected = res.total;
          }
          if (status === DataMap.detectionSnapshotStatus.detecting.value) {
            this.totalDetecting = res.total;
          }
          if (status === DataMap.detectionSnapshotStatus.nodetecte.value) {
            this.totalNodetecte = res.total;
          }
        } else {
          this.totalResource = res.total;
        }
      });
  }

  getSnapshot(mask = true) {
    const params: any = {
      pageNo: CommonConsts.PAGE_START,
      pageSize: CommonConsts.PAGE_SIZE,
      akLoading: mask
    };
    this.copiesDetectReportService
      .ShowDetectionDetails(params)
      .subscribe(res => {
        this.totalSnapshot = res.total;
      });
  }

  refreshResource() {
    this.getResource(null, false);
    this.getResource(DataMap.detectionSnapshotStatus.infected.value, false);
    this.getResource(DataMap.detectionSnapshotStatus.uninfected.value, false);
    this.getResource(DataMap.detectionSnapshotStatus.detecting.value, false);
  }

  refreshSnapshot() {
    this.getSnapshot(false);
  }

  refresh() {
    if (this.selectedViewType === '0') {
      this.ResourceListComponent.getResource();
    } else {
      this.SnapshotListComponent.getSnapshot();
    }
  }

  search(name) {
    if (this.selectedViewType === '0') {
      this.ResourceListComponent.getResource(name);
    } else {
      this.SnapshotListComponent.getSnapshot(name);
    }
  }

  selectionChange(selection) {
    this.deleteBtnDisable =
      isEmpty(selection) ||
      !isEmpty(
        find(selection, item =>
          includes(
            [DataMap.detectionSnapshotStatus.detecting.value],
            item.anti_status
          )
        )
      );
  }

  deleteCopyData() {
    const timeArr = map(this.SnapshotListComponent.selection, item => {
      return this.datePipe.transform(
        item.display_timestamp,
        'yyyy-MM-dd HH:mm:ss'
      );
    });
    this.warningMessageService.create({
      content: this.i18n.get('common_snapshot_delete_label', [
        timeArr.join(',')
      ]),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.copiesApiService.deleteCopyV1CopiesCopyIdCyberDelete({
              copyId: item.uuid,
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          map(cloneDeep(this.SnapshotListComponent.selection), item => {
            return assign(item, {
              name: this.datePipe.transform(
                item.display_timestamp,
                'yyyy-MM-dd HH:mm:ss'
              ),
              isAsyn: true
            });
          }),
          () => {
            this.SnapshotListComponent.selection = [];
            this.deleteBtnDisable = true;
            this.SnapshotListComponent.getSnapshot();
          },
          '',
          true
        );
      }
    });
  }
}
