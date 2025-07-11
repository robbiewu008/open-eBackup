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
  Component,
  EventEmitter,
  Input,
  OnInit,
  Output,
  TemplateRef,
  ViewChild
} from '@angular/core';
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
  RestoreFilesControllerService,
  RestoreLocationType,
  RestoreManagerService as RestoreService,
  RestoreType,
  VirtualResourceService,
  VmFileReplaceStrategy,
  VmRestoreOptionType
} from 'app/shared';
import { each, size, first, includes, trim, isFunction, assign } from 'lodash';
import { map } from 'rxjs/operators';

@Component({
  selector: 'aui-file-restore',
  templateUrl: './file-restore.component.html',
  styleUrls: ['./file-restore.component.less']
})
export class FileRestoreComponent implements OnInit {
  @Input() childResType;
  @Input() rowCopy;
  @Input() fileLevelRestore;
  @Output() restoreParamsChange = new EventEmitter<any>();
  DataMap = DataMap;
  formGroup: FormGroup;
  resourceProperties;
  properties;
  originVmIp;
  newVmIp;
  selectedVm;
  restoreLocationType = RestoreLocationType;
  dataMap = DataMap;
  RestoreType = RestoreType;
  disableOriginLocation = false;

  vmFileReplaceStrategy = VmFileReplaceStrategy;
  VmRestoreOptionType = VmRestoreOptionType;
  selectedPathData = [];

  vmIpOptions = [];
  vmIpNoData = false;
  isTest = false;
  disabled = true;
  os_type;

  pageSize = CommonConsts.PAGE_SIZE_SMALL;

  vmIpTypeOptions = {
    exist: '1',
    custom: '2'
  };

  vmIpHelpLabel = this.i18n.get('protection_file_restore_ip_help_label');
  pathErrorTip = {
    invalidPath: this.i18n.get('common_path_error_label')
  };
  ipErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.ipErrorTip
  };
  portErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidInteger: this.i18n.get('common_valid_integer_label'),
    invalidRang: this.i18n.get('common_valid_rang_label', [1, 65535])
  };

  @ViewChild('footerTpl', { static: true }) footerTpl: TemplateRef<any>;

  constructor(
    private i18n: I18NService,
    private fb: FormBuilder,
    private modal: ModalRef,
    private baseUtilService: BaseUtilService,
    private restoreService: RestoreService,
    private virtualResourceService: VirtualResourceService,
    private restoreFilesControllerService: RestoreFilesControllerService
  ) {}

  ngOnInit() {
    this.initFooter();
    this.initData();
    this.initForm();
  }

  initFooter() {
    this.modal.setProperty({ lvFooter: this.footerTpl });
  }

  initData() {
    this.disableOriginLocation = includes(
      [
        DataMap.CopyData_generatedType.replicate.value,
        DataMap.CopyData_generatedType.reverseReplication.value,
        DataMap.CopyData_generatedType.cascadedReplication.value
      ],
      this.rowCopy?.generated_by
    );
    each(this.rowCopy.fineGrainedData, item => {
      this.selectedPathData.push({
        name: item
      });
    });
    this.resourceProperties = JSON.parse(this.rowCopy.resource_properties);
    this.properties = JSON.parse(this.rowCopy.properties);
  }

  validTargetPath(): ValidatorFn {
    return (control: AbstractControl): { [key: string]: any } | null => {
      if (!trim(control.value)) {
        return null;
      }
      if (
        !CommonConsts.REGEX.windowsPath.test(control.value) &&
        !CommonConsts.REGEX.linuxPath.test(control.value)
      ) {
        return { invalidPath: { value: control.value } };
      }
      return null;
    };
  }

  setPortValid(updateValid = true) {
    if (this.showPort()) {
      this.formGroup
        .get('port')
        ?.addValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.integer(),
          this.baseUtilService.VALID.rangeValue(1, 65535)
        ]);
    } else {
      this.formGroup.get('port')?.clearValidators();
    }
    if (updateValid) {
      this.formGroup.get('port')?.updateValueAndValidity();
    }
  }

  initForm() {
    this.formGroup = this.fb.group({
      restoreLocation: [RestoreLocationType.ORIGIN],
      location: new FormControl(this.resourceProperties.path, {
        validators: [this.baseUtilService.VALID.required()]
      }),
      targetPath: new FormControl('', {
        validators: [this.validTargetPath()]
      }),
      originalType: [VmFileReplaceStrategy.Overwriting],
      vmIpType: new FormControl(this.vmIpTypeOptions.exist),
      vmIp: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      customIp: new FormControl(''),
      port: new FormControl('22'),
      userName: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      password: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      })
    });

    this.formGroup
      .get('vmIpType')
      .valueChanges.subscribe(res => this.listenVmIpType(res));

    if (this.disableOriginLocation) {
      this.formGroup.patchValue({
        restoreLocation: RestoreLocationType.NEW,
        location: ''
      });
    } else {
      this.getVirtualResource().subscribe((res: any) => {
        this.os_type = res?.os_type;
        this.getVmIpOptions(res.vm_ip);
      });
      this.setPortValid(false);
    }

    this.listenRestoreLocation();

    this.formGroup.valueChanges.subscribe(res => {
      this.isTest = false;
    });

    this.formGroup.statusChanges.subscribe(res =>
      this.restoreParamsChange.emit(res)
    );
  }

  listenVmIpType(res) {
    if (res === this.vmIpTypeOptions.exist) {
      this.formGroup
        .get('vmIp')
        .setValidators([this.baseUtilService.VALID.required()]);
      this.formGroup.get('customIp').clearValidators();
    } else {
      this.formGroup
        .get('customIp')
        .setValidators([
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.ip()
        ]);
      this.formGroup.get('vmIp').clearValidators();
    }
    this.formGroup.get('customIp').updateValueAndValidity();
    this.formGroup.get('vmIp').updateValueAndValidity();
  }

  listenRestoreLocation() {
    this.formGroup.get('restoreLocation').valueChanges.subscribe(res => {
      if (res === RestoreLocationType.ORIGIN) {
        this.newVmIp = this.formGroup.value.vmIp;
        this.formGroup.patchValue({
          location: this.resourceProperties.path,
          vmIp: this.originVmIp
        });
        this.getVirtualResource().subscribe((vm: any) => {
          this.getVmIpOptions(vm.vm_ip);
        });
      } else {
        this.originVmIp = this.formGroup.value.vmIp;
        this.formGroup.patchValue({
          location: this.selectedVm && this.selectedVm.path,
          vmIp: this.newVmIp
        });
        if (this.selectedVm) {
          this.getVmIpOptions(this.selectedVm.vmIp);
        } else {
          this.vmIpOptions = [];
          this.vmIpNoData = false;
        }
      }
      this.setPortValid();
      this.formGroup.get('vmIp').markAsPristine();
      this.formGroup.get('vmIp').markAsUntouched();
    });
  }

  changeLocation(event) {
    this.selectedVm = event;
    this.formGroup.patchValue({ location: event.path, vmIp: '' });
    this.formGroup.get('vmIp').markAsPristine();
    this.formGroup.get('vmIp').markAsUntouched();
    this.getVmIpOptions(event.vmIp);
    // 增加端口校验
    this.setPortValid();
  }

  changeVcenter(event) {
    this.vmIpHelpLabel = this.i18n.get(
      'protection_file_restore_ip_help_label',
      [event?.label || '']
    );
  }

  getVmIpOptions(vmIp) {
    this.vmIpOptions = [];
    if (!vmIp) {
      this.vmIpNoData = true;
      return;
    } else {
      this.vmIpNoData = false;
    }
    const vmIpList = vmIp.split(',');
    vmIpList.forEach(ip => {
      this.vmIpOptions.push({
        key: ip,
        label: ip,
        isLeaf: true
      });
    });
  }

  getVirtualResource() {
    return this.virtualResourceService
      .queryResourcesV1VirtualResourceGet({
        pageSize: CommonConsts.PAGE_SIZE,
        pageNo: CommonConsts.PAGE_START,
        conditions: JSON.stringify({
          uuid: this.rowCopy.resource_id
        })
      })
      .pipe(map(res => (!!size(res.items) ? first(res.items) : {})));
  }

  getVmIp(): string {
    return this.formGroup.get('vmIpType')?.value === this.vmIpTypeOptions.exist
      ? this.formGroup.get('vmIp')?.value
      : this.formGroup.get('customIp')?.value;
  }

  getParams() {
    const params = {
      copy_id: this.rowCopy.uuid,
      object_type: this.rowCopy.resource_sub_type,
      restore_location: this.formGroup.value.restoreLocation,
      filters: [],
      restore_objects: this.rowCopy.fineGrainedData,
      restore_type: RestoreType.FileRestore,
      target: {
        details: [],
        env_id:
          this.formGroup.value.restoreLocation === this.restoreLocationType.NEW
            ? this.selectedVm.uuid
            : this.rowCopy.resource_id,
        env_type: DataMap.Resource_Type.virtualMachine.value,
        restore_target:
          this.formGroup.value.restoreLocation === this.restoreLocationType.NEW
            ? `${this.selectedVm?.path}${this.selectedVm?.name}`
            : ''
      },
      source: {
        source_location: this.rowCopy.resource_location,
        source_name: this.rowCopy.resource_name
      },
      ext_parameters: {
        vm_name:
          this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN
            ? this.resourceProperties.name
            : this.selectedVm.name,
        USER_NAME: this.formGroup.value.userName,
        PASSWORD: this.formGroup.value.password,
        VM_IP: this.getVmIp(),
        FILE_REPLACE_STRATEGY: this.formGroup.value.originalType,
        TARGET_PATH: this.formGroup.value.targetPath || ''
      }
    };
    if (this.showPort()) {
      assign(params.ext_parameters, {
        PORT: this.formGroup.get('port')?.value
      });
    }
    return params;
  }

  showPort(): boolean {
    if (this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN) {
      return (
        this.os_type === DataMap.vmwareOsType.linux.value ||
        this.properties?.osType === DataMap.vmwareOsType.linux.value
      );
    }
    return this.selectedVm?.os_type === DataMap.vmwareOsType.linux.value;
  }

  testConnection() {
    const params = {
      username: this.formGroup.value.userName,
      password: this.formGroup.value.password,
      osType:
        this.formGroup.value.restoreLocation === RestoreLocationType.ORIGIN
          ? this.os_type || this.properties?.osType || ''
          : this.selectedVm.os_type || '',
      vmIp: this.getVmIp()
    };
    if (this.showPort()) {
      assign(params, {
        port: this.formGroup.get('port')?.value
      });
    }
    this.restoreFilesControllerService
      .checkDestConnection({ checkDestConnectionRequest: params })
      .subscribe({
        next: () => {
          this.isTest = true;
        },
        error: () => {
          this.isTest = false;
        }
      });
  }

  restore() {
    const params = this.getParams();
    this.restoreService
      .createRestoreV1RestoresPost({ body: params })
      .subscribe(res => {
        this.modal.close();
        if (isFunction(this.rowCopy.closeDetailFn)) {
          this.rowCopy.closeDetailFn();
        }
      });
  }
}
