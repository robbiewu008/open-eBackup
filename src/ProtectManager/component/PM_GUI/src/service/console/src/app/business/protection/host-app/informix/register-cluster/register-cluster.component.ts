import { Component, Input, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import {
  BaseUtilService,
  DataMap,
  getMultiHostOps,
  I18NService,
  MultiCluster,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType
} from 'app/shared';
import { cacheGuideResource } from 'app/shared/consts/guide-config';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { each, filter, get, isEmpty, size, trim } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-register-cluster',
  templateUrl: './register-cluster.component.html',
  styleUrls: ['./register-cluster.component.less']
})
export class RegisterClusterComponent implements OnInit {
  dataDetail;
  optsConfig;
  optItems = [];
  hostOptions = [];
  dataMap = DataMap;

  tableData = {
    data: [],
    total: 0
  };
  formGroup: FormGroup;
  regex: RegExp = /^[^/].*|^(\/[^\/]+\/?)$/;
  specialRegex: RegExp = /^(\/mnt\/databackup|\/opt\/DataBackup)/;
  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    pathError: this.i18n.get('common_path_error_label'),
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128])
  };
  @Input() rowData;
  constructor(
    private appUtilsService: AppUtilsService,
    private fb: FormBuilder,
    public i18n: I18NService,
    public baseUtilService: BaseUtilService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      host: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      logBackup: new FormControl(false),
      logBackupPath: new FormControl('')
    });

    this.formGroup.get('logBackup').valueChanges.subscribe(res => {
      if (res) {
        this.formGroup
          .get('logBackupPath')
          .setValidators([
            this.baseUtilService.VALID.required(),
            this.validPath(),
            this.baseUtilService.VALID.maxLength(128)
          ]);
      } else {
        this.formGroup.get('logBackupPath').clearValidators();
      }

      this.formGroup.get('logBackupPath').updateValueAndValidity();
    });

    if (this.rowData) {
      this.getDataDetail();
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

  getDataDetail() {
    this.protectedResourceApiService
      .ShowResource({ resourceId: this.rowData.uuid })
      .subscribe(res => {
        this.formGroup.patchValue({
          name: res.name,
          host: get(res, 'dependencies.agents[0].uuid'),
          logBackup: res.extendInfo?.logBackup === '1' ? true : false,
          logBackupPath: get(res, 'extendInfo.logBackupPath', '')
        });
        this.formGroup.get('name').disable();
        this.dataDetail = res;
      });
  }

  getProxyOptions() {
    const extParams = {
      conditions: JSON.stringify({
        type: 'Plugin',
        subType: [`${DataMap.Resource_Type.informixService.value}Plugin`]
      })
    };
    this.appUtilsService.getResourceByRecursion(
      extParams,
      params => this.protectedResourceApiService.ListResources(params),
      resource => {
        resource = filter(resource, item => !isEmpty(item.environment));
        const hostArray = [];
        resource = filter(
          resource,
          item =>
            item.environment.extendInfo.scenario ===
            DataMap.proxyHostType.external.value
        );
        if (MultiCluster.isMulti && isEmpty(this.rowData)) {
          resource = getMultiHostOps(resource);
        }
        resource.sort(
          (a, b) => b.environment.linkStatus - a.environment.linkStatus
        );
        each(resource, item => {
          const tmp = item.environment;
          hostArray.push({
            ...tmp,
            key: tmp.uuid,
            value: tmp.uuid,
            label: `${tmp.name}(${tmp.endpoint})`,
            isLeaf: true,
            disabled:
              tmp.linkStatus !==
              DataMap.resource_LinkStatus_Special.normal.value
          });
        });
        this.hostOptions = hostArray;
      }
    );
  }

  getParams() {
    return {
      name: this.formGroup.get('name').value,
      type: ResourceType.DATABASE,
      subType: DataMap.Resource_Type.informixService.value,
      extendInfo: {
        hostId: this.formGroup.value.host,
        logBackup: this.formGroup.value.logBackup ? '1' : '0',
        logBackupPath: this.formGroup.value.logBackup
          ? this.formGroup.value.logBackupPath
          : ''
      },
      dependencies: {
        agents: [
          {
            uuid: this.formGroup.value.host
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
          .subscribe({
            next: res => {
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      } else {
        this.protectedEnvironmentApiService
          .RegisterProtectedEnviroment({
            RegisterProtectedEnviromentRequestBody: params
          })
          .subscribe({
            next: res => {
              cacheGuideResource(res);
              observer.next();
              observer.complete();
            },
            error: err => {
              observer.error(err);
              observer.complete();
            }
          });
      }
    });
  }
}
