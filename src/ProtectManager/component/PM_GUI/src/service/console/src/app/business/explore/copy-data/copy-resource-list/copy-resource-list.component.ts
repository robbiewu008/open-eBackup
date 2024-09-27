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
import { Component, Input, OnInit, ViewChild } from '@angular/core';
import { MessageService } from '@iux/live';
import {
  CookieService,
  DataMap,
  GROUP_COMMON,
  WormStatusEnum,
  hasCopyDeletePermission
} from 'app/shared';
import { CopiesService } from 'app/shared/api/services';
import { I18NService, WarningMessageService } from 'app/shared/services';
import { BatchOperateService } from 'app/shared/services/batch-operate.service';
import {
  assign,
  cloneDeep,
  find,
  includes,
  intersection,
  isNil,
  isUndefined,
  map
} from 'lodash';

@Component({
  selector: 'aui-copy-resource-list',
  templateUrl: './copy-resource-list.component.html',
  styleUrls: ['./copy-resource-list.component.less'],
  providers: [DatePipe]
})
export class CopyResourceListComponent implements OnInit {
  deleteBtnDisable = true;
  selectedViewType = '0';
  isResourceReplicaList = !this.cookieService.isCloudBackup;
  resType = DataMap.Resource_Type;
  _intersection = intersection;

  groupCommon = GROUP_COMMON;

  @Input() header;
  @Input() resourceType;
  @Input() childResourceType;
  @ViewChild('resourceList', { static: false }) resourceListComponent;
  @ViewChild('replicaResourceList', { static: false })
  replicaResourceListComponent;
  @ViewChild('copyList', { static: false }) copyListComponent;

  constructor(
    public i18n: I18NService,
    public datePipe: DatePipe,
    public copiesApiService: CopiesService,
    public warningMessageService: WarningMessageService,
    private messageService: MessageService,
    public batchOperateService: BatchOperateService,
    private cookieService: CookieService
  ) {}

  _isWorm(row) {
    return [
      WormStatusEnum.SETTING,
      WormStatusEnum.SET_SUCCESS,
      WormStatusEnum.SET_FAILED
    ].includes(row?.worm_status);
  }

  deleteCopyData() {
    if (
      !isNil(
        find(
          this.copyListComponent.selection,
          item =>
            this._isWorm(item) &&
            item.backup_type !== DataMap.CopyData_Backup_Type.snapshot.value
        )
      )
    ) {
      this.messageService.error(
        this.i18n.get('explore_worm_policy_error_del_label'),
        {
          lvMessageKey: 'lvMsg_key_explore_worm_policy_error_del_label',
          lvShowCloseButton: true
        }
      );
      return;
    }
    const timeArr = map(this.copyListComponent.selection, item => {
      return this.datePipe.transform(
        item.display_timestamp,
        'yyyy-MM-dd HH:mm:ss'
      );
    });
    this.warningMessageService.create({
      content: this.i18n.get('common_copy_delete_label', [timeArr.join(',')]),
      onOK: () => {
        this.batchOperateService.selfGetResults(
          item => {
            return this.copiesApiService.deleteCopyV1CopiesCopyIdDelete({
              copyId: item.uuid,
              akDoException: false,
              akOperationTips: false,
              akLoading: false
            });
          },
          map(cloneDeep(this.copyListComponent.selection), item => {
            return assign(item, {
              name: this.datePipe.transform(
                item.display_timestamp,
                'yyyy-MM-dd HH:mm:ss'
              ),
              isAsyn: true
            });
          }),
          () => {
            this.copyListComponent.getCopies();
          }
        );
      }
    });
  }

  selectionChange(selection) {
    this.deleteBtnDisable =
      !selection.length ||
      !isUndefined(
        find(
          selection,
          item =>
            !includes(
              [
                DataMap.copydata_validStatus.normal.value,
                DataMap.copydata_validStatus.invalid.value,
                DataMap.copydata_validStatus.deleteFailed.value
              ],
              item.status
            ) ||
            item.generated_by ===
              DataMap.CopyData_generatedType.tapeArchival.value ||
            item.backup_type === DataMap.CopyData_Backup_Type.log.value ||
            !hasCopyDeletePermission(item)
        )
      );
  }

  ngOnInit() {
    this.viewChange(this.selectedViewType);
  }

  onChange() {
    this.refresh();
  }

  viewChange(val) {
    this.selectedViewType = val;
  }

  search(value) {
    if (this.isResourceReplicaList) {
      this.replicaResourceListComponent.resourceName = value;
      this.replicaResourceListComponent.searchByName(value);
    } else {
      this.resourceListComponent.resourceName = value;
      this.resourceListComponent.searchByName(value);
    }
  }

  refresh() {
    if (this.selectedViewType === '0') {
      if (this.isResourceReplicaList) {
        this.replicaResourceListComponent.getResource();
      } else {
        this.resourceListComponent.getResource();
      }
      return;
    }
    this.copyListComponent.getCopies();
  }
}
