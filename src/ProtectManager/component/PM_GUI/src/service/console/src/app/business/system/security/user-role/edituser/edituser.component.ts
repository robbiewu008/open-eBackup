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
import { Component, OnInit } from '@angular/core';
import {
  FormBuilder,
  FormGroup,
  FormControl,
  ValidatorFn,
  AbstractControl
} from '@angular/forms';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  WarningMessageService
} from 'app/shared';
import { UsersApiService } from 'app/shared/api/services';
import { DrawModalService } from 'app/shared/services/draw-modal.service';
import { Observable, Observer } from 'rxjs';
import { assign, includes, isNaN, trim } from 'lodash';
import { ModalRef } from '@iux/live';

@Component({
  selector: 'edituser',
  templateUrl: './edituser.component.html',
  styleUrls: ['./edituser.component.less']
})
export class EdituserComponent implements OnInit {
  user;
  sessionRequired;
  limitPlaceholder = '1~8';
  formGroup: FormGroup;
  rangeErrorTip = assign(
    { ...this.baseUtilService.integerErrorTip },
    { ...this.baseUtilService.rangeErrorTip },
    {
      invalidRang: this.i18n.get('common_valid_rang_label', [1, 8])
    }
  );

  dataMap = DataMap;
  methodTypeOptions = this.dataMapService.toArray('loginMethod').map(item => {
    item.isLeaf = true;
    return item;
  });
  emailErrorTip = assign(
    {},
    this.baseUtilService.requiredErrorTip,
    {
      invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [254])
    },
    {
      invalidEmail: this.i18n.get('system_error_email_label')
    }
  );

  isOceanProtect = !includes(
    [
      DataMap.Deploy_Type.cloudbackup2.value,
      DataMap.Deploy_Type.cloudbackup.value,
      DataMap.Deploy_Type.hyperdetect.value,
      DataMap.Deploy_Type.cyberengine.value
    ],
    this.i18n.get('deploy_type')
  );

  constructor(
    public i18n: I18NService,
    public fb: FormBuilder,
    public modal: ModalRef,
    public userApiService: UsersApiService,
    public baseUtilService: BaseUtilService,
    public drawModalService: DrawModalService,
    public warningMessageService: WarningMessageService,
    private dataMapService: DataMapService
  ) {}

  ngOnInit() {
    this.initForm();
  }

  initForm() {
    this.formGroup = this.fb.group({
      loginType: new FormControl(this.user.loginType),
      dynamicCodeEmail: new FormControl(this.user.dynamicCodeEmail),
      description: new FormControl({
        value: this.i18n.get(this.user.description),
        disabled: this.user.defaultUser
      }),
      sessionControl: this.user.sessionControl,
      sessionLimit: this.user.sessionControl
        ? new FormControl(this.user.sessionLimit, {
            validators: [
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.user.userType === DataMap.loginUserType.saml.value
                ? this.baseUtilService.VALID.rangeValue(1, 100)
                : this.baseUtilService.VALID.rangeValue(1, 8)
            ],
            updateOn: 'change'
          })
        : '',
      neverExpire: new FormControl(this.user?.neverExpire)
    });

    if (this.user.userType === DataMap.loginUserType.saml.value) {
      this.rangeErrorTip.invalidRang = this.i18n.get(
        'common_valid_rang_label',
        [1, 100]
      );
      this.limitPlaceholder = '1~100';
    }

    this.sessionRequired = this.user.sessionControl;
    setTimeout(() => {
      this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
    }, 0);
    this.listenForm();
  }

  listenForm() {
    if (!this.isOceanProtect) {
      return;
    }
    if (this.user.loginType === DataMap.loginMethod.email.value) {
      this.formGroup
        .get('dynamicCodeEmail')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.validEmail()
        ]);
    }
    this.formGroup.get('dynamicCodeEmail').updateValueAndValidity();
    this.formGroup.get('loginType').valueChanges.subscribe(res => {
      if (res === DataMap.loginMethod.email.value) {
        this.formGroup
          .get('dynamicCodeEmail')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validEmail()
          ]);
      } else {
        this.formGroup.get('dynamicCodeEmail').clearValidators();
      }
      this.formGroup.get('dynamicCodeEmail').updateValueAndValidity();
    });
  }

  validEmail(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!control.value) {
        return null;
      }
      const email = control.value;
      if (!CommonConsts.REGEX.email.test(email)) {
        return { invalidEmail: { value: control.value } };
      }
      if (email.split('@')[1] && email.split('@')[1].length > 255) {
        return { invalidEmail: { value: control.value } };
      }
      return null;
    };
  }

  sessionControlChange(sessionControl) {
    const sessionLimitCtrl = this.formGroup.get('sessionLimit');
    if (sessionControl) {
      sessionLimitCtrl.setValidators([
        this.baseUtilService.VALID.required(),
        this.baseUtilService.VALID.integer(),
        this.user.userType === DataMap.loginUserType.saml.value
          ? this.baseUtilService.VALID.rangeValue(1, 100)
          : this.baseUtilService.VALID.rangeValue(1, 8)
      ]);
      sessionLimitCtrl.setValue(
        this.user.sessionControl ? this.user.sessionLimit : ''
      );
    } else {
      sessionLimitCtrl.clearValidators();
      sessionLimitCtrl.setValue('');
    }
    this.sessionRequired = sessionControl;
    sessionLimitCtrl.updateValueAndValidity();
  }

  sessionLimitBlur() {
    if (
      !isNaN(+this.formGroup.value.sessionLimit) &&
      trim(this.formGroup.value.sessionLimit) !== ''
    ) {
      this.formGroup.patchValue({
        sessionLimit: +this.formGroup.value.sessionLimit
      });
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (!this.formGroup.value.sessionControl) {
        delete this.formGroup.value.sessionLimit;
      }

      if (this.user.rolesSet[0].roleId !== 7) {
        this.formGroup.get('neverExpire').setValue(false);
      }

      const params = {
        userId: this.user.userId,
        userRequest: {
          ...this.formGroup.value,
          resourceSetAuthorizationSets: [
            { roleId: this.user.rolesSet[0].roleId, resourceSetIds: [] }
          ]
        }
      };
      if (!this.isOceanProtect) {
        delete params.userRequest.loginType;
        delete params.userRequest.dynamicCodeEmail;
      }
      this.userApiService.updateUsingPUT(params).subscribe({
        next: () => {
          observer.next();
          observer.complete();
        },
        error: error => {
          observer.error(error);
          observer.complete();
        }
      });
    });
  }
}
