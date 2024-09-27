import { ModalRef } from '@iux/live';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import {
  I18NService,
  BaseUtilService,
  DataMap,
  ProtectedEnvironmentApiService,
  ProtectedResourceApiService,
  ResourceType,
  InstanceType
} from 'app/shared';
import { cloneDeep, each, find, isUndefined, set, size } from 'lodash';

@Component({
  selector: 'aui-add-host',
  templateUrl: './add-host.component.html',
  styleUrls: ['./add-host.component.less']
})
export class AddHostComponent implements OnInit {
  rowData;
  name;
  parentUuid;
  data = [];
  children = [];
  isTest = false;
  okLoading = false;
  testLoading = false;
  hostOptions = [];
  formGroup: FormGroup;

  portErrorTip = {
    ...this.baseUtilService.rangeErrorTip,
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };
  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };
  passwordErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  constructor(
    private fb: FormBuilder,
    public modal: ModalRef,
    private i18n: I18NService,
    private protectedResourceApiService: ProtectedResourceApiService,
    private protectedEnvironmentApiService: ProtectedEnvironmentApiService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.getProxyOptions();
  }

  initForm() {
    this.formGroup = this.fb.group({
      host: new FormControl([], {
        validators: [this.baseUtilService.VALID.required()]
      }),
      userName: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      }),
      password: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ],
        updateOn: 'change'
      })
    });

    if (this.rowData) {
      this.formGroup.patchValue({
        host: [this.rowData.extendInfo?.hostId],
        userName: this.rowData.auth?.authKey
      });
      this.formGroup.get('userName').disable();
    }
  }

  getProxyOptions() {
    const params = {
      resourceId: this.parentUuid
    };
    this.protectedResourceApiService.ShowResource(params).subscribe(res => {
      const hostArray = [];
      each(res['dependencies']['agents'], item => {
        hostArray.push({
          ...item,
          key: item.uuid,
          value: item.uuid,
          label: `${item.name}(${item.endpoint})`,
          isLeaf: true
        });
      });
      if (this.rowData) {
        this.hostOptions = [
          find(
            hostArray,
            item => item.value === this.rowData.extendInfo?.hostId
          )
        ];
        return;
      }
      if (!!size(this.children)) {
        this.hostOptions = hostArray.filter(item =>
          isUndefined(
            find(
              this.children,
              child => child.extendInfo?.hostId === item.value
            )
          )
        );
        return;
      }
      this.hostOptions = hostArray;
    });
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      let index = size(this.children);
      each(this.formGroup.value.host, item => {
        const host = find(this.hostOptions, { uuid: item });
        if (this.rowData) {
          this.data = [cloneDeep(this.rowData)];

          set(this.data[0], 'auth', {
            authType: DataMap.Database_Auth_Method.db.value,
            authKey: this.formGroup.get('userName').value,
            authPwd: this.formGroup.value.password
          });
        } else {
          this.data.push({
            parentUuid: '',
            name: this.name,
            type: ResourceType.DATABASE,
            subType: DataMap.Resource_Type.dbTwoInstance.value,
            extendInfo: {
              hostId: item,
              isTopInstance: InstanceType.NotTopinstance
            },
            dependencies: {
              agents: [{ uuid: item }]
            },
            auth: {
              authType: DataMap.Database_Auth_Method.db.value,
              authKey: this.formGroup.get('userName').value,
              authPwd: this.formGroup.value.password
            },
            hostName: host?.name,
            ip: host?.endpoint
          });
        }
      });
      observer.next();
      observer.complete();
    });
  }
}
