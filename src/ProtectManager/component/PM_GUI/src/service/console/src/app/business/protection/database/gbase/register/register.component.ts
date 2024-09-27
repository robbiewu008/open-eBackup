import { Component, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { filter, get, map, size, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-gbase-register',
  templateUrl: './register.component.html',
  styleUrls: ['./register.component.less']
})
export class RegisterComponent implements OnInit {
  rowData;
  formGroup: FormGroup;
  hostOptions = [];
  regex: RegExp = /^[^/].*|^(\/[^\/]+\/?)$/;
  specialRegex: RegExp = /^(\/mnt\/databackup|\/opt\/DataBackup)/;

  nodeErrorTip = {
    required: this.i18n.get('common_host_number_least_2_label'),
    invalidMinLength: this.i18n.get('common_host_number_least_2_label')
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128])
  };

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private appUtilsService: AppUtilsService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.getHostOptions();
    this.updateForm();
  }

  getHostOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.gbaseCluster.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(
          resource,
          item =>
            item.environment &&
            item.environment.extendInfo?.scenario ===
              DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti) {
          resource = filter(resource, item => {
            const val = item.environment;
            const connection = val?.extendInfo?.connection_result;
            const target = JSON.parse(connection)[MultiCluster.esn];
            if (target?.link_status) {
              return true;
            }
            return (
              val.linkStatus ===
              DataMap.resource_LinkStatus_Special.normal.value
            );
          });
        }
        this.hostOptions = map(resource, item => {
          return {
            ...item.environment,
            key: item.environment.endpoint,
            value: item.environment.uuid,
            label: `${item.environment.name}(${item.environment.endpoint})`,
            isLeaf: true
          };
        });
      }
    );
  }

  disableBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.name(CommonConsts.REGEX.name),
          this.baseUtilService.VALID.maxLength(32)
        ]
      }),
      node: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      logBackup: new FormControl(false),
      path: new FormControl('')
    });

    this.formGroup.get('logBackup').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('path')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validPath(),
            this.baseUtilService.VALID.maxLength(128)
          ]);
      } else {
        this.formGroup.get('path').clearValidators();
      }

      this.formGroup.get('path').updateValueAndValidity();
    });

    this.formGroup.statusChanges.subscribe(() => this.disableBtn());
  }

  updateForm() {
    if (this.rowData?.uuid) {
      this.protectedResourceApiService
        .ShowResource({ resourceId: this.rowData.uuid })
        .subscribe(res => {
          this.formGroup.patchValue({
            name: res.name,
            node: get(res, 'dependencies.agents[0].uuid'),
            logBackup: res.extendInfo?.logBackup === '1',
            logBackupPath: get(res, 'extendInfo.logBackupPath', '')
          });
        });
    }
  }

  validPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (
        !trim(control.value) ||
        size(trim(control.value)) !== size(control.value)
      ) {
        return { pathError: { value: control.value } };
      }
      const paths = control.value;

      if (this.regex.test(paths) || this.specialRegex.test(paths)) {
        return { pathError: { value: control.value } };
      }
      return null;
    };
  }

  getParams() {
    return {
      name: this.formGroup.value.name,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.gbaseCluster.value,
      extendInfo: {
        hostId: this.formGroup.value.node,
        logBackup: this.formGroup.value.logBackup ? '1' : '0',
        logBackupPath: this.formGroup.value.logBackup
          ? this.formGroup.value.path
          : ''
      },
      dependencies: {
        agents: [
          {
            uuid: this.formGroup.value.node
          }
        ]
      }
    };
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }
      const params = this.getParams();
      if (this.rowData) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowData.uuid,
            UpdateProtectedEnvironmentRequestBody: params
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
          })
          .subscribe(
            () => {
              observer.next();
              observer.complete();
            },
            err => {
              observer.error(err);
              observer.complete();
            }
          );
      }
    });
  }
}
