import { Component, OnDestroy, OnInit } from '@angular/core';
import {
  AbstractControl,
  FormBuilder,
  FormControl,
  FormGroup,
  ValidatorFn
} from '@angular/forms';
import { MessageService, ModalRef, UploadFile } from '@iux/live';
import {
  BaseUtilService,
  CommonConsts,
  DataMap,
  DataMapService,
  I18NService,
  ProtectedEnvironmentApiService
} from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import {
  assign,
  each,
  first,
  isEmpty,
  isString,
  isUndefined,
  last,
  reject,
  size,
  toNumber,
  uniqueId
} from 'lodash';
import { Observable, Observer, Subject } from 'rxjs';
import { takeUntil } from 'rxjs/operators';

@Component({
  selector: 'aui-create-cluster',
  templateUrl: './create-cluster.component.html',
  styleUrls: ['./create-cluster.component.less']
})
export class CreateClusterComponent implements OnInit, OnDestroy {
  rowItem: any;
  formGroup: FormGroup;
  dataMap = DataMap;
  isEn = this.i18n.isEn;
  configFileFilter;
  selectConfigFile;
  configFiles = [];
  _isEn = this.i18n.isEn;
  clusterOptions = this.dataMapService
    .toArray('k8sClusterType')
    .filter(item => {
      return (item.isLeaf = true);
    });

  _destory = new Subject();

  includeLabels = [];
  prefixInKey = 'prefixInKey';
  prefixInValue = 'prefixInValue';
  kubeconfigHelp = this.i18n.get('protetion_kubernetes_config_help_label');
  tagHelp = this.i18n.get('protetion_kubernetes_tag_help_label');
  tokenHelp = this.i18n.get('protetion_kubernetes_token_help_label');

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  tokenErrorTip = {
    ...this.baseUtilService.requiredErrorTip
  };
  taskNumberErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 8])
  };
  podTagErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidPodTag: this.i18n.get('protection_kubernetes_pod_tag_vaild_label')
  };
  keyErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidLabel: this.i18n.get('protection_labels_key_valid_label')
  };
  valueErrorTip = {
    invalidMaxLength: this.i18n.get('protection_labels_value_valid_label'),
    invalidName: this.i18n.get('protection_labels_value_valid_label')
  };

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private i18n: I18NService,
    private message: MessageService,
    private appUtilsService: AppUtilsService,
    public baseUtilService: BaseUtilService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    private dataMapService: DataMapService
  ) {}

  ngOnDestroy() {
    this._destory.next(true);
    this._destory.complete();
  }

  ngOnInit() {
    this.initForm();
    this.initConfigFileFilter();
    this.updateForm();
  }

  configHelpHover() {
    const url1 = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000001839260265.html'
      : '/console/assets/help/a8000/zh-cn/index.html#zh-cn_topic_0000001839260265.html';

    const url2 = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000001917330525.html'
      : '/console/assets/help/a8000/zh-cn/index.html#zh-cn_topic_0000001917330525.html';
    this.appUtilsService.openSpecialHelp([url1, url2]);
  }

  tagHelpHover() {
    const url = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000001940484365.html'
      : '/console/assets/help/a8000/zh-cn/index.html#zh-cn_topic_0000001940484365.html';
    this.appUtilsService.openSpecialHelp(url);
  }

  tokenHelpHover() {
    const url = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000001839180449.html'
      : '/console/assets/help/a8000/zh-cn/index.html#zh-cn_topic_0000001839180449.html';
    this.appUtilsService.openSpecialHelp(url);
  }

  updateForm() {
    if (isEmpty(this.rowItem)) {
      return;
    }
    let taskTimeout;
    try {
      taskTimeout = JSON.parse(this.rowItem.extendInfo?.taskTimeout);
    } catch (error) {
      taskTimeout = {};
    }
    this.formGroup.patchValue({
      type: this.rowItem.auth?.authType,
      name: this.rowItem.name,
      ip: this.rowItem.endpoint,
      port: this.rowItem.port,
      clusterType: this.rowItem.extendInfo?.clusterType,
      podTag: this.rowItem.extendInfo?.imageNameAndTag,
      taskNumber: this.rowItem.extendInfo?.jobNumOnSingleNode,
      cert: this.rowItem.extendInfo?.isVerifySsl === '1',
      timeoutDay: taskTimeout?.days ?? 1,
      timeoutHour: taskTimeout?.hours ?? 0,
      timeoutMin: taskTimeout?.minutes ?? 0,
      timeoutSec: taskTimeout?.seconds ?? 0
    });
    if (
      isString(this.rowItem.extendInfo?.nodeSelector) &&
      !isEmpty(this.rowItem.extendInfo?.nodeSelector)
    ) {
      each(this.rowItem.extendInfo?.nodeSelector.split(','), item => {
        this.addIncludeLabels(item.split('=')[0], item.split('=')[1]);
      });
    }
  }

  addIncludeLabels(key?: string, value?: string) {
    const id = uniqueId();
    this.formGroup.addControl(
      this.prefixInKey + id,
      new FormControl(key || '', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.appUtilsService.validLabel()
        ]
      })
    );
    this.formGroup.addControl(
      this.prefixInValue + id,
      new FormControl(value || '', {
        validators: [
          this.baseUtilService.VALID.name(CommonConsts.REGEX.label, false),
          this.baseUtilService.VALID.maxLength(63)
        ]
      })
    );
    this.includeLabels.push({ id });
    this.formGroup.updateValueAndValidity();
  }

  deleteIncludeLabels(id) {
    this.includeLabels = reject(this.includeLabels, v => v.id === id);
    this.formGroup.removeControl(this.prefixInKey + id);
    this.formGroup.removeControl(this.prefixInValue + id);
    this.formGroup.updateValueAndValidity();
  }

  getIncludeLabels() {
    if (isEmpty(this.includeLabels)) {
      return '';
    }
    const arr = [];
    each(this.includeLabels, item => {
      arr.push(
        `${this.formGroup.get(`${this.prefixInKey}${item.id}`)?.value}=${
          this.formGroup.get(`${this.prefixInValue}${item.id}`)?.value
        }`
      );
    });
    return arr.join(',');
  }

  initForm() {
    this.formGroup = this.fb.group({
      type: new FormControl(DataMap.Cluster_Register_Mode.token.value),
      name: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.name(),
          this.baseUtilService.VALID.maxLength(64)
        ]
      }),
      ip: new FormControl(
        { value: '', disabled: !!this.rowItem?.uuid },
        {
          validators: [
            this.baseUtilService.VALID.required(),
            this.baseUtilService.VALID.ip()
          ]
        }
      ),
      port: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]
      }),
      clusterType: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      token: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      podTag: new FormControl('', {
        validators: [this.baseUtilService.VALID.required(), this.validPodTag()]
      }),
      taskNumber: new FormControl(4, {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 8)
        ]
      }),
      timeoutDay: new FormControl(1),
      timeoutHour: new FormControl(0),
      timeoutMin: new FormControl(0),
      timeoutSec: new FormControl(0),
      cert: new FormControl(true),
      certData: new FormControl('')
    });
    if (isEmpty(this.rowItem)) {
      this.formGroup
        .get('certData')
        .setValidators([this.baseUtilService.VALID.required()]);
    }
    this.listenForm();
  }

  validPodTag(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (isUndefined(this.formGroup)) {
        return null;
      }

      if (!control.value) {
        return { required: { value: control.value } };
      }

      const values = control.value.split(':');
      if (values.length < 2) {
        return { invalidPodTag: { value: control.value } };
      }
      const reg = /^[a-zA-Z0-9][a-zA-Z0-9_.-]{0,61}[a-zA-Z0-9]$|^[a-zA-Z0-9]$/;

      if (isEmpty(first(values)) && values.length === 2) {
        return { invalidPodTag: { value: control.value } };
      }

      if (!reg.test(last(values))) {
        return { invalidPodTag: { value: control.value } };
      }
      return null;
    };
  }

  listenForm() {
    this.formGroup
      .get('type')
      .valueChanges.pipe(takeUntil(this._destory))
      .subscribe(res => {
        if (res === DataMap.Cluster_Register_Mode.token.value) {
          this.formGroup
            .get('ip')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.ip()
            ]);
          this.formGroup
            .get('port')
            .setValidators([
              this.baseUtilService.VALID.required(),
              this.baseUtilService.VALID.integer(),
              this.baseUtilService.VALID.rangeValue(1, 65535)
            ]);
          this.formGroup
            .get('token')
            .setValidators([this.baseUtilService.VALID.required()]);
          if (this.formGroup.value.cert) {
            this.formGroup
              .get('certData')
              .setValidators([this.baseUtilService.VALID.required()]);
          } else {
            this.formGroup.get('certData').clearValidators();
          }
        } else {
          this.formGroup.get('ip').clearValidators();
          this.formGroup.get('port').clearValidators();
          this.formGroup.get('token').clearValidators();
          this.formGroup.get('certData').clearValidators();
        }
        this.formGroup.get('ip').updateValueAndValidity();
        this.formGroup.get('port').updateValueAndValidity();
        this.formGroup.get('token').updateValueAndValidity();
        this.formGroup.get('certData').updateValueAndValidity();
      });
    this.formGroup
      .get('cert')
      .valueChanges.pipe(takeUntil(this._destory))
      .subscribe(res => {
        if (
          res &&
          DataMap.Cluster_Register_Mode.token.value ===
            this.formGroup.value.type
        ) {
          this.formGroup
            .get('certData')
            .setValidators([this.baseUtilService.VALID.required()]);
        } else {
          this.formGroup.get('certData').clearValidators();
        }
        this.formGroup.get('certData').updateValueAndValidity();
      });
    this.formGroup.statusChanges
      .pipe(takeUntil(this._destory))
      .subscribe(() => this.setOkDisabled());
  }

  initConfigFileFilter() {
    this.configFileFilter = [
      {
        name: 'suffix',
        filterFn: (files: UploadFile[]) => {
          if (files[0].size > 1024 * 1024) {
            this.message.error(
              this.i18n.get('common_max_size_file_label', ['1MB']),
              {
                lvMessageKey: 'maxSizeFileErrorKey3',
                lvShowCloseButton: true
              }
            );
            this.selectConfigFile = '';
            this.setOkDisabled();
            return [];
          }
          const reader = new FileReader();
          reader.onloadend = () => {
            this.selectConfigFile = (reader.result as any)
              .replace('data:', '')
              .replace(/^.+,/, '');
            this.setOkDisabled();
          };
          reader.readAsDataURL(files[0].originFile);
          return files;
        }
      }
    ];
  }

  configFileChange(files) {
    if (size(files) === 0) {
      this.selectConfigFile = '';
      this.setOkDisabled();
    }
  }

  setOkDisabled() {
    this.modal.getInstance().lvOkDisabled =
      this.formGroup.value.type === DataMap.Cluster_Register_Mode.token.value
        ? this.formGroup.invalid
        : this.formGroup.invalid || isEmpty(this.selectConfigFile);
  }

  getParams() {
    const params = {
      name: this.formGroup.value.name,
      type: 'KubernetesCommon',
      subType: DataMap.Resource_Type.kubernetesClusterCommon.value,
      extendInfo: {
        clusterType: this.formGroup.value.clusterType,
        imageNameAndTag: this.formGroup.value.podTag,
        isVerifySsl: this.formGroup.value.cert ? '1' : '0',
        taskTimeout: JSON.stringify({
          days: Number(this.formGroup.value.timeoutDay),
          hours: Number(this.formGroup.value.timeoutHour),
          minutes: Number(this.formGroup.value.timeoutMin),
          seconds: Number(this.formGroup.value.timeoutSec)
        })
      }
    };
    if (!isEmpty(this.includeLabels)) {
      assign(params.extendInfo, {
        nodeSelector: this.getIncludeLabels()
      });
    } else {
      assign(params.extendInfo, {
        nodeSelector: ''
      });
    }
    if (this.formGroup.value.taskNumber) {
      assign(params.extendInfo, {
        jobNumOnSingleNode: toNumber(this.formGroup.value.taskNumber)
      });
    }
    if (
      this.formGroup.value.type === DataMap.Cluster_Register_Mode.token.value
    ) {
      assign(params, {
        endpoint: this.formGroup.value.ip || this.rowItem?.endpoint,
        port: toNumber(this.formGroup.value.port),
        auth: {
          authType: DataMap.Cluster_Register_Mode.token.value,
          extendInfo: {
            token: this.formGroup.value.token,
            certificateAuthorityData: this.formGroup.value.cert
              ? this.formGroup.value.certData
              : ''
          }
        }
      });
    } else {
      assign(params, {
        auth: {
          authType: DataMap.Cluster_Register_Mode.kubeconfig.value,
          extendInfo: {
            configKey: this.selectConfigFile
          }
        }
      });
    }
    return params;
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        observer.error(null);
        observer.complete();
        return;
      }
      const params = this.getParams();
      if (this.rowItem?.uuid) {
        this.protectedEnvironmentApiService
          .UpdateProtectedEnvironment({
            envId: this.rowItem.uuid,
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
