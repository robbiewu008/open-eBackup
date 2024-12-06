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
import { Component, Input, OnDestroy, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import {
  ApplicationType,
  CommonConsts,
  DataMap,
  PolicyType
} from 'app/shared/consts';
import { CookieService, GlobalService, I18NService } from 'app/shared/services';
import { each, find, includes, isBoolean, isEmpty, isUndefined } from 'lodash';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-update-index',
  templateUrl: './update-index.component.html',
  styleUrls: ['./update-index.component.less']
})
export class UpdateIndexComponent implements OnInit, OnDestroy {
  @Input() formGroup: FormGroup;
  @Input() isDetail;
  @Input() protectData;
  @Input() extParams;

  initSla = false;
  hasBackup = false;
  hasArchiveForObs = false;
  hasArchiveForTape = false;
  isHcsUser = this.cookieService.get('userType') === CommonConsts.HCS_USER_TYPE;
  _isBoolean = isBoolean;
  autoIndexForObs = false;
  autoIndexForTape = false; // 磁带库下支持自动索引的应用

  destroy$ = new Subject();

  constructor(
    private i18n: I18NService,
    private cookieService: CookieService,
    private globalService: GlobalService
  ) {}

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  ngOnInit(): void {
    this.getState();
  }

  getAutoIndexTip(appType) {
    // 获取自动索引问号提示内容
    if (appType === ApplicationType.HDFS) {
      return this.i18n.get('protection_hdfs_auto_index_tip_label');
    } else {
      return this.i18n.get('protection_auto_index_tip_label');
    }
  }

  getState(): void {
    this.globalService
      .getState('slaSelectedEvent')
      .pipe(takeUntil(this.destroy$))
      .subscribe(res => {
        this.initSla = true;
        this.getSlaDetail(res);
        this.updateAutoIndex();
        this.getAutoIndexTip(res.application);
      });
  }

  updateAutoIndex() {
    if (!this.extParams) {
      return;
    }
    // 索引
    each(
      [
        'backup_res_auto_index',
        'archive_res_auto_index',
        'tape_archive_auto_index'
      ],
      key => {
        if (isBoolean(this.extParams[key])) {
          if (this.formGroup.get(key)) {
            this.formGroup.get(key).setValue(this.extParams[key]);
          } else {
            this.formGroup.addControl(
              key,
              new FormControl(this.extParams[key])
            );
          }
        }
      }
    );
  }

  setAutoIndex(backupPolicy) {
    const indexKey = !isUndefined(
      backupPolicy.ext_parameters.fine_grained_restore
    )
      ? 'fine_grained_restore'
      : 'auto_index';

    if (!this.formGroup?.get('backup_res_auto_index')) {
      this.formGroup.addControl(
        'backup_res_auto_index',
        new FormControl(backupPolicy.ext_parameters[indexKey])
      );
    } else {
      this.formGroup
        ?.get('backup_res_auto_index')
        .setValue(backupPolicy.ext_parameters[indexKey]);
    }
  }

  getSlaDetail(sla): void {
    if (!sla) {
      return;
    }
    this.autoIndexForObs = includes(
      [
        ApplicationType.NASFileSystem,
        ApplicationType.NASShare,
        ApplicationType.HDFS,
        ApplicationType.ImportCopy,
        ApplicationType.Fileset
      ],
      sla.application
    );
    this.autoIndexForTape = includes(
      [
        ApplicationType.NASFileSystem,
        ApplicationType.NASShare,
        ApplicationType.Fileset,
        ApplicationType.ObjectStorage
      ],
      sla.application
    );
    const backupPolicy = find(sla.policy_list, { type: PolicyType.BACKUP });
    // 当前只判断对象存储归档
    const archivePolicyForObs = find(
      sla.policy_list,
      item =>
        item.type === PolicyType.ARCHIVING &&
        item.ext_parameters?.protocol ===
          DataMap.Archival_Protocol.objectStorage.value
    );

    // 当前只判断磁带库存储归档
    const archivePolicyForTape = find(
      sla.policy_list,
      item =>
        item.type === PolicyType.ARCHIVING &&
        item.ext_parameters?.protocol ===
          DataMap.Archival_Protocol.tapeLibrary.value
    );
    // 备份自动索引
    this.hasBackup =
      !isEmpty(backupPolicy) &&
      (!isUndefined(backupPolicy.ext_parameters.fine_grained_restore) ||
        !isUndefined(backupPolicy.ext_parameters.auto_index));

    if (this.hasBackup) {
      this.setAutoIndex(backupPolicy);
    }
    // 对象存储归档自动索引
    this.hasArchiveForObs =
      this.autoIndexForObs &&
      !isEmpty(archivePolicyForObs) &&
      !isUndefined(archivePolicyForObs.ext_parameters.auto_index);

    if (this.hasArchiveForObs) {
      if (!this.formGroup?.get('archive_res_auto_index')) {
        this.formGroup.addControl(
          'archive_res_auto_index',
          new FormControl(archivePolicyForObs.ext_parameters.auto_index)
        );
      } else {
        this.formGroup
          ?.get('archive_res_auto_index')
          .setValue(archivePolicyForObs.ext_parameters.auto_index);
      }
    }

    // 磁带库归档自动索引，暂时屏蔽
    this.hasArchiveForTape = false;
    if (this.hasArchiveForTape) {
      if (!this.formGroup?.get('tape_archive_auto_index')) {
        this.formGroup.addControl(
          'tape_archive_auto_index',
          new FormControl(archivePolicyForTape?.ext_parameters?.auto_index)
        );
      } else {
        this.formGroup
          ?.get('tape_archive_auto_index')
          .setValue(archivePolicyForTape?.ext_parameters?.auto_index);
      }
    }
  }
}
