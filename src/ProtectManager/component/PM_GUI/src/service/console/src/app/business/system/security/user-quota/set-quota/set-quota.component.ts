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
  ChangeDetectionStrategy,
  ChangeDetectorRef,
  Component,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup
} from '@angular/forms';
import {
  BaseUtilService,
  CAPACITY_UNIT,
  DataMap,
  DataMapService,
  I18NService,
  KerberosAPIService,
  WarningMessageService
} from 'app/shared';
import { ProTableComponent } from 'app/shared/components/pro-table';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { VirtualScrollService } from 'app/shared/services/virtual-scroll.service';
import {
  defer,
  each,
  filter as _filter,
  get,
  isEmpty,
  toLower,
  toNumber
} from 'lodash';

@Component({
  selector: 'aui-set-quota',
  templateUrl: './set-quota.component.html',
  styleUrls: ['./set-quota.component.less'],
  changeDetection: ChangeDetectionStrategy.OnPush
})
export class SetQuotaComponent implements OnInit {
  item;
  setType;
  showSamlQuotaTips = false;
  formGroup: FormGroup;
  dataMap = DataMap;
  unitconst = CAPACITY_UNIT;
  unitOptions = this.dataMapService.toArray('Capacity_Unit').map(item => {
    item['isLeaf'] = true;
    return item;
  });

  capacityErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_positive_integer_label'),
    invalidSize: this.i18n.get('system_quota_size_invalid_label'),
    invalidMaxSize: this.i18n.get('common_valid_maxsize_label', ['256PB'])
  };

  @ViewChild('dataTable', { static: false }) dataTable: ProTableComponent;
  @ViewChild('capacity', { static: true })
  capacity: TemplateRef<any>;

  constructor(
    public i18n: I18NService,
    public drawModalService: DrawModalService,
    public kerberosApi: KerberosAPIService,
    public cdr: ChangeDetectorRef,
    public virtualScroll: VirtualScrollService,
    public warningMessageService: WarningMessageService,
    public dataMapService: DataMapService,
    private fb: FormBuilder,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateData();
  }

  initForm() {
    this.formGroup = this.fb.group({
      backupQuota: new FormControl(false),
      archiveQuota: new FormControl(false),
      backupCapacity: new FormControl(''),
      backupCapacityUnit: new FormControl(DataMap.Capacity_Unit.kb.value),
      archiveCapacity: new FormControl(''),
      archiveCapacityUnit: new FormControl(DataMap.Capacity_Unit.kb.value)
    });

    this.formGroup.get('backupQuota').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('backupCapacity')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.integer(),
            this.compareBackupCapacitySize()
          ]);
      } else {
        this.formGroup.get('backupCapacity').clearValidators();
      }
      this.formGroup.get('backupCapacity').updateValueAndValidity();
    });

    this.formGroup.get('archiveQuota').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('archiveCapacity')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.integer(),
            this.compareArchiveCapacitySize()
          ]);
      } else {
        this.formGroup.get('archiveCapacity').clearValidators();
      }
      this.formGroup.get('archiveCapacity').updateValueAndValidity();
    });

    this.formGroup.get('backupCapacityUnit').valueChanges.subscribe(res => {
      defer(() => {
        this.formGroup
          .get('backupCapacity')
          .setValue(this.formGroup.value.backupCapacity);
      });
    });

    this.formGroup.get('archiveCapacityUnit').valueChanges.subscribe(res => {
      defer(() => {
        this.formGroup
          .get('archiveCapacity')
          .setValue(this.formGroup.value.archiveCapacity);
      });
    });
  }

  updateData() {
    if (this.item?.userType === DataMap.loginUserType.saml.value) {
      if (this.setType === DataMap.userFunction.backup.value) {
        this.formGroup.get('backupQuota').setValue(true);
      } else {
        this.formGroup.get('archiveQuota').setValue(true);
      }
    } else {
      if (this.setType === DataMap.userFunction.backup.value) {
        this.formGroup
          .get('backupQuota')
          .setValue(this.item?.backupTotalQuota !== -1);
      } else {
        this.formGroup
          .get('archiveQuota')
          .setValue(this.item?.cloudArchiveTotalQuota !== -1);
      }
    }

    let backupUnit;
    let archiveUnit;
    each(this.unitOptions, (unit, idx) => {
      if (this.item?.backupTotalQuota % unit.convertByte !== 0) {
        backupUnit = backupUnit
          ? backupUnit
          : this.unitOptions[idx ? idx - 1 : idx];
      }
      if (this.item?.cloudArchiveTotalQuota % unit.convertByte !== 0) {
        archiveUnit = archiveUnit
          ? archiveUnit
          : this.unitOptions[idx ? idx - 1 : idx];
      }
      if (
        this.item?.backupTotalQuota / unit.convertByte >= 1 &&
        this.item?.backupTotalQuota / unit.convertByte < 1024
      ) {
        backupUnit = backupUnit ? backupUnit : unit;
      }
      if (
        this.item?.cloudArchiveTotalQuota / unit.convertByte >= 1 &&
        this.item?.cloudArchiveTotalQuota / unit.convertByte < 1024
      ) {
        archiveUnit = archiveUnit ? archiveUnit : unit;
      }
    });

    if (!isEmpty(backupUnit)) {
      this.formGroup
        .get('backupCapacity')
        .setValue(
          this.item?.backupTotalQuota === -1
            ? ''
            : this.item?.backupTotalQuota / backupUnit.convertByte
        );
      this.formGroup.get('backupCapacityUnit').setValue(backupUnit.value);
    }

    if (!isEmpty(archiveUnit)) {
      this.formGroup
        .get('archiveCapacity')
        .setValue(
          this.item?.cloudArchiveTotalQuota === -1
            ? ''
            : this.item?.cloudArchiveTotalQuota / archiveUnit.convertByte
        );
      this.formGroup.get('archiveCapacityUnit').setValue(archiveUnit.value);
    }
  }

  integer() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      return !/^[1-9]\d*$/.test(control.value)
        ? { invalidInteger: { value: control.value } }
        : null;
    };
  }

  compareBackupCapacitySize() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      const quotaSize =
        toNumber(control.value) *
        <any>(
          get(
            DataMap.Capacity_Unit,
            `${toLower(this.formGroup.value.backupCapacityUnit)}.convertByte`
          )
        );

      if (quotaSize > 256 * DataMap.Capacity_Unit.pb.convertByte) {
        return { invalidMaxSize: { value: control.value } };
      }

      if (quotaSize < this.item?.backupUsedQuota) {
        if (this.item.userType === DataMap.loginUserType.saml.value) {
          this.showSamlQuotaTips = true;
          return null;
        } else {
          return { invalidSize: { value: control.value } };
        }
      } else {
        this.showSamlQuotaTips = false;
        return null;
      }
    };
  }

  compareArchiveCapacitySize() {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }

      const quotaSize =
        toNumber(control.value) *
        <any>(
          get(
            DataMap.Capacity_Unit,
            `${toLower(this.formGroup.value.archiveCapacityUnit)}.convertByte`
          )
        );

      if (quotaSize > 256 * DataMap.Capacity_Unit.pb.convertByte) {
        return { invalidMaxSize: { value: control.value } };
      }

      if (quotaSize < this.item?.cloudArchiveUsedQuota) {
        if (this.item.userType === DataMap.loginUserType.saml.value) {
          this.showSamlQuotaTips = true;
          return null;
        } else {
          return { invalidSize: { value: control.value } };
        }
      } else {
        this.showSamlQuotaTips = false;
        return null;
      }
    };
  }
}
