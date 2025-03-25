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
  AntiRansomwareInfectConfigApiService,
  AntiRansomwarePolicyApiService,
  BaseUtilService,
  GlobalService,
  I18NService
} from 'app/shared';
import { DataMap, PolicyType, RouterUrl } from 'app/shared/consts';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { find, isBoolean } from 'lodash';
import { Subject, takeUntil } from 'rxjs';

@Component({
  selector: 'aui-copy-limit-advanced-parameter',
  templateUrl: './copy-limit-advanced-parameter.component.html',
  styleUrls: ['./copy-limit-advanced-parameter.component.less']
})
export class CopyLimitAdvancedParameterComponent implements OnInit, OnDestroy {
  @Input() data;
  @Input() formGroup: FormGroup;
  @Input() hasArchive = false; // 复制副本判断sla有没有归档策略
  @Input() slaData; // 复制副本判断sla归档策略配置
  @Input() isDetail; //是否是详情
  @Input() protectData;
  hasWorm = false;
  wormName;
  needWorm = false;
  securityTip = this.i18n.get('explore_security_archive_cycle_label');
  _isBoolean = isBoolean;
  isDistributed = this.appUtilsService.isDistributed;

  destroy$ = new Subject();

  constructor(
    public baseUtilService: BaseUtilService,
    public appUtilsService: AppUtilsService,
    private i18n: I18NService,
    private globalService: GlobalService,
    private antiRansomwarePolicyApiService: AntiRansomwarePolicyApiService,
    private antiRansomwareInfectedCopyService: AntiRansomwareInfectConfigApiService
  ) {}

  ngOnInit() {
    if (this.isDetail || this.isDistributed) {
      return;
    }
    this.initForm();
    this.getWormStatus();
    this.getState();
    if (this.hasArchive && !!this.slaData) {
      this.parseSlaData(this.slaData);
    }
  }

  ngOnDestroy(): void {
    this.destroy$.next(true);
    this.destroy$.complete();
  }

  parseSlaData(res?) {
    const tmpArchivePolicy = find(res.policy_list, {
      type: PolicyType.ARCHIVING
    });
    this.needWorm = !(
      tmpArchivePolicy.schedule.trigger ===
        DataMap.Archive_Trigger.immediatelyBackup.value &&
      tmpArchivePolicy.ext_parameters.archiving_scope ===
        DataMap.Archive_Scope.latest.value
    );
    this.formGroup.get('enable_security_archive').updateValueAndValidity();

    if (
      tmpArchivePolicy.schedule.trigger ===
      DataMap.Archive_Trigger.immediatelyBackup.value
    ) {
      this.securityTip = this.i18n.get(
        'explore_security_archive_non_cycle_label'
      );
    } else {
      this.securityTip = this.i18n.get('explore_security_archive_cycle_label');
    }
  }

  getState(): void {
    window.addEventListener('storage', event => {
      if (
        event.key === 'addWormComplete' &&
        event.newValue === '1' &&
        event?.oldValue !== '1'
      ) {
        this.getWormStatus();
      }
    });
    if (this.hasArchive) {
      return;
    }
    this.globalService
      .getState('slaSelectedEvent')
      .pipe(takeUntil(this.destroy$))
      .subscribe(res => {
        this.hasArchive = !!find(res.policy_list, {
          type: PolicyType.ARCHIVING
        });
        if (this.hasArchive) {
          this.parseSlaData(res);
        } else {
          this.formGroup.get('enable_security_archive').setValue(false);
          this.formGroup.get('hasWorm').clearValidators();
          this.formGroup.get('hasWorm').updateValueAndValidity();
        }
      });
  }

  initForm() {
    this.formGroup.addControl('hasWorm', new FormControl(''));
    this.formGroup.addControl(
      'enable_security_archive',
      new FormControl(false)
    );

    this.formGroup
      .get('enable_security_archive')
      .valueChanges.subscribe(res => {
        if (this.needWorm && res) {
          this.formGroup
            .get('hasWorm')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('hasWorm').clearValidators();
        }
        this.formGroup.get('hasWorm').updateValueAndValidity();
      });
    if (
      this.data?.protectedObject ||
      !!this.data?.ext_parameters ||
      (!!this.data?.resourceId && !!this.data?.sla_id)
    ) {
      this.updateData();
    }
  }

  updateData() {
    const extParameters =
      this.data?.protectedObject?.extParameters || this.data?.ext_parameters;
    this.formGroup.patchValue({
      enable_security_archive: extParameters?.enable_security_archive
    });
    this.formGroup.updateValueAndValidity();
  }

  getWormStatus() {
    this.antiRansomwarePolicyApiService
      .ShowAntiRansomwarePolicies({
        resourceIds: [this.data?.uuid || this.data?.resourceId]
      })
      .subscribe(res => {
        this.hasWorm = !!res?.records?.length;
        localStorage.removeItem('addWormComplete');
        if (this.hasWorm) {
          this.wormName = res.records[0].policyName;
          this.formGroup.get('hasWorm').setValue(true);
        } else {
          this.formGroup.get('hasWorm').setValue(false);
        }
      });
  }

  jumpToWorm() {
    window.open(
      `/console/#${RouterUrl.ExplorePolicyAntiPolicySetting}`,
      '_blank'
    );
  }
}
