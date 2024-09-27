import { Component, Input, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { ModalRef } from '@iux/live';
import {
  RestoreApiV2Service,
  RestoreType,
  RestoreV2LocationType,
  RestoreV2Type
} from 'app/shared';
import { get } from 'lodash';
import { Observable, Observer } from 'rxjs';

@Component({
  selector: 'aui-database-restore',
  templateUrl: './restore-database.component.html',
  styleUrls: ['./restore-database.component.less']
})
export class DatabaseRestoreComponent implements OnInit {
  @Input() rowCopy;
  @Input() childResType;
  @Input() restoreType;
  formGroup: FormGroup;
  resourceData;

  constructor(
    private fb: FormBuilder,
    private modal: ModalRef,
    private restoreV2Service: RestoreApiV2Service
  ) {}

  ngOnInit() {
    this.resourceData = JSON.parse(this.rowCopy.resource_properties) || {};
    this.initForm();
    this.disableOkBtn();
  }

  initForm() {
    this.formGroup = this.fb.group({
      originCluster: new FormControl({
        value:
          this.rowCopy?.resource_environment_name ||
          this.rowCopy?.resource_location,
        disabled: true
      })
    });
  }

  getParams() {
    const params = {
      copyId: this.rowCopy.uuid,
      targetEnv: get(this.resourceData, 'root_uuid'),
      restoreType:
        this.restoreType === RestoreType.CommonRestore
          ? RestoreV2Type.CommonRestore
          : RestoreV2Type.FileRestore,
      targetLocation: RestoreV2LocationType.ORIGIN,
      extendInfo: {
        resourceSubType: this.rowCopy.resource_sub_type
      }
    };
    return params;
  }

  getTargetParams() {
    return {
      ...this.formGroup.value,
      resource: {
        name:
          this.rowCopy?.resource_environment_name ||
          this.rowCopy?.resource_location,
        value: get(this.resourceData, 'root_uuid')
      },
      restoreLocation: RestoreV2LocationType.ORIGIN,
      requestParams: this.getParams()
    };
  }

  getTargetPath() {
    return (
      this.rowCopy?.resource_environment_name || this.rowCopy?.resource_location
    );
  }

  restore(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      const params = this.getParams();
      this.restoreV2Service
        .CreateRestoreTask({ CreateRestoreTaskRequestBody: params as any })
        .subscribe(
          res => {
            observer.next();
            observer.complete();
          },
          err => {
            observer.error(err);
            observer.complete();
          }
        );
    });
  }

  disableOkBtn() {
    this.modal.getInstance().lvOkDisabled = this.formGroup.invalid;
  }
}
