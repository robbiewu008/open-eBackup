import { DatePipe } from '@angular/common';
import { Component, OnInit, TemplateRef, ViewChild } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageboxService, MessageService, ModalRef } from '@iux/live';
import {
  BaseUtilService,
  DataMap,
  I18NService,
  SysbackupApiService,
  CommonConsts,
  SystemApiService
} from 'app/shared';
import { assign, includes, isUndefined, trim, isFunction } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'policy-config',
  templateUrl: './policy-config.component.html',
  providers: [DatePipe]
})
export class PolicyConfigComponent implements OnInit {
  data;
  callBack;
  formGroup: FormGroup;
  keepCountErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [3, 20])
  });
  encryptedPwdErrorTip = assign({}, this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  });
  destUsernameErrorTip = assign({}, this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255]),
    invalidSpecailChars: this.i18n.get('system_invalid_sftp_name_label')
  });
  destPwdErrorTip = assign({}, this.baseUtilService.pwdErrorTip, {
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [255])
  });
  destPortErrorTip = assign({}, this.baseUtilService.rangeErrorTip, {
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  });
  destPathErrorTip = {
    invalidName: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [1024])
  };
  encryptionPasswordTip = this.i18n.get(
    'system_encryption_password_help_label'
  );
  pwdComplexTip = this.i18n.get('common_pwdtip_label', [
    8,
    15,
    this.i18n.get('common_pwd_complex_label'),
    2,
    ''
  ]);
  isCyberengine =
    this.i18n.get('deploy_type') === DataMap.Deploy_Type.cyberengine.value;

  @ViewChild('tipContentTpl', { static: false }) tipContentTpl: TemplateRef<
    any
  >;
  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  constructor(
    public modal: ModalRef,
    public i18n: I18NService,
    public fb: FormBuilder,
    private message: MessageService,
    public datePipe: DatePipe,
    private messageBox: MessageboxService,
    public baseUtilService: BaseUtilService,
    private systemApiService: SystemApiService,
    private sysbackupApiService: SysbackupApiService
  ) {}

  ngOnInit() {
    this.getFooter();
    this.initForm();
    this.updateData();
    this.setDestPath();
  }

  getFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  updateData() {
    this.formGroup.patchValue({
      ...this.data,
      backupTime: this.data.backupTime
        ? new Date(`2020/10/10 ${this.data.backupTime}:00`)
        : '',
      keepCount: !this.data.keepCount ? '7' : this.data.keepCount,
      destPort:
        this.data.destPort === '0'
          ? this.isCyberengine
            ? '22'
            : ''
          : this.data.destPort,
      destPath: this.isCyberengine ? '' : this.data.destPath
    });
  }

  setDestPath() {
    if (
      !this.isCyberengine ||
      Object.prototype.toString.call(this.data?.destPath) !== '[object String]'
    ) {
      return;
    }
    this.systemApiService.queryEsnUsingGET({ akDoException: false }).subscribe(
      res => {
        this.formGroup
          .get('destPath')
          .setValue(this.data?.destPath.replace(`/${res.esn}`, ''));
      },
      () => {
        this.formGroup.get('destPath').setValue(this.data.destPath);
      }
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      destType: new FormControl(DataMap.System_Backup_DestType.sftp.value),
      defaultPolicy: new FormControl(
        DataMap.System_Backup_DefaultPolicy.default
      ),
      destIp: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip(),
          this.validSpecialIp()
        ],
        updateOn: 'change'
      }),
      destUsername: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(255),
          this.validUserName()
        ],
        updateOn: 'change'
      }),
      destPwd: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(255)
        ],
        updateOn: 'change'
      }),
      destPort: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ],
        updateOn: 'change'
      }),
      destPath: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.maxLength(1024),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.linuxPath, false)
        ],
        updateOn: 'change'
      }),
      backupTime: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()],
        updateOn: 'change'
      }),
      keepCount: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(3, 20)
        ],
        updateOn: 'change'
      }),
      backupPwd: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.password(8, 2, 15),
          this.validConfirmPwdIsSame()
        ],
        updateOn: 'change'
      }),
      confirmPassword: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.password(8, 2, 15),
          this.validNewPwdIsSame()
        ],
        updateOn: 'change'
      })
    });
  }

  validSpecialIp(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (includes(['127.0.0.1', '0.0.0.0'], trim(control.value))) {
        return { invalidName: { value: control.value } };
      }

      return null;
    };
  }

  validUserName(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      const reg = /[':\?\\"<>\|\*;`\$&\-\(\)#\!]+/;
      const isValid = !reg.test(control.value);
      if (isValid) {
        return null;
      }

      return { invalidSpecailChars: { value: control.value } };
    };
  }

  validNewPwdIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value.backupPwd &&
        this.formGroup.value.backupPwd !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.formGroup.value.backupPwd) {
        this.formGroup.get('backupPwd').setErrors(null);
      }

      return null;
    };
  }

  validConfirmPwdIsSame(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (
        !!this.formGroup.value.confirmPassword &&
        this.formGroup.value.confirmPassword !== control.value
      ) {
        return { diffPwd: { value: control.value } };
      }

      if (!!this.formGroup.value.confirmPassword) {
        this.formGroup.get('confirmPassword').setErrors(null);
      }

      return null;
    };
  }

  onOk(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      if (this.data.id) {
        this.messageBox.info({
          lvHeader: this.i18n.get('common_alarms_info_label'),
          lvContent: this.tipContentTpl,
          lvFooter: [
            {
              label: this.i18n.get('common_ok_label'),
              onClick: modal => {
                modal.close();
                this.modifyBackupPolicy(observer);
              }
            },
            {
              label: this.i18n.get('common_cancel_label'),
              onClick: modal => {
                modal.close();
                observer.error({});
                observer.complete();
              }
            }
          ],
          lvAfterClose: modal => {
            if (modal && modal.trigger === 'close') {
              observer.next();
              observer.complete();
            }
          }
        });
        return;
      }

      this.modifyBackupPolicy(observer);
    });
  }

  ok() {
    this.onOk().subscribe(() => {
      this.modal.close();
      if (isFunction(this.callBack)) {
        this.callBack();
      }
    });
  }

  modifyBackupPolicy(observer: Observer<void>) {
    const params = {
      ...this.data,
      ...this.formGroup.value,
      backupTime: this.datePipe.transform(
        this.formGroup.value.backupTime,
        'HH:mm'
      ),
      destPath: this.formGroup.value.destPath
        ? this.formGroup.value.destPath
        : null
    };
    this.sysbackupApiService
      .modifyPolicyUsingPOST({
        systemBackupPolicyRequest: params
      })
      .subscribe(
        () => {
          observer.next();
          observer.complete();
        },
        error => {
          observer.error(error);
          observer.complete();
        }
      );
  }

  test() {
    const params = {
      ip: this.formGroup.get('destIp').value,
      username: this.formGroup.get('destUsername').value,
      password: this.formGroup.get('destPwd').value,
      port: this.formGroup.get('destPort').value,
      destPath: this.formGroup.value.destPath
        ? this.formGroup.value.destPath
        : null
    };
    this.sysbackupApiService
      .testSftpConnection({ request: params })
      .subscribe(() => {});
  }
}
