import { ModalRef } from '@iux/live';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import {
  I18NService,
  BaseUtilService,
  DataMap,
  DataMapService
} from 'app/shared';
import { cloneDeep, filter, find, get, includes, set, toNumber } from 'lodash';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-add-host',
  templateUrl: './add-host.component.html',
  styleUrls: ['./add-host.component.less']
})
export class AddHostComponent implements OnInit {
  rowData;
  subType;
  name;
  parentUuid;
  data = [];
  children = [];
  authOptions = this.dataMapService
    .toArray('Database_Auth_Method')
    .map(item => {
      item['isLeaf'] = true;
      return item;
    })
    .filter(item => {
      return item.value === DataMap.Database_Auth_Method.os.value;
    });
  hostOptions = [];
  formGroup: FormGroup;

  nameErrorTip = {
    ...this.baseUtilService.nameErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [64])
  };
  pathErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [128])
  };

  constructor(
    public modal: ModalRef,
    private fb: FormBuilder,
    public i18n: I18NService,
    public dataMapService: DataMapService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.hostOptions = filter(this.hostOptions, host => {
      return !find(
        this.children,
        child =>
          host.endpoint === get(child, 'endpoint') &&
          host.endpoint !== get(this.rowData, 'endpoint')
      );
    });
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.resource, item.uuid)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      name: new FormControl('', {
        validators: [this.baseUtilService.VALID.name()]
      }),
      host: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      authMode: new FormControl(DataMap.Database_Auth_Method.os.value),
      sqlhost: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      }),
      onconfig: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(128)
        ]
      })
    });

    if (this.rowData) {
      const parent = find(this.hostOptions, item => {
        return item.endpoint === this.rowData.endpoint;
      });
      this.formGroup.patchValue({
        name: get(this.rowData, 'name'),
        host: parent.uuid,
        authMode: get(this.rowData, 'auth.authType'),
        sqlhost: get(this.rowData, 'extendInfo.sqlhostsPath'),
        onconfig: get(this.rowData, 'extendInfo.onconfigPath')
      });
      this.formGroup.get('name').disable();
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const host = find(this.hostOptions, { uuid: this.formGroup.value.host });

      if (this.rowData) {
        this.data = [cloneDeep(this.rowData)];

        set(this.data[0], 'extendInfo', {
          onconfigPath: this.formGroup.value.onconfig,
          sqlhostsPath: this.formGroup.value.sqlhost
        });
      } else {
        this.data.push({
          name: this.formGroup.value.name,
          host: get(host, 'name'),
          endpoint: get(host, 'endpoint'),
          linkStatus: toNumber(get(host, 'linkStatus')),
          parentUuid: this.formGroup.value.host,
          auth: {
            authType: DataMap.Database_Auth_Method.os.value
          },
          dependencies: {
            agents: [
              {
                uuid: this.formGroup.value.host
              }
            ]
          },
          extendInfo: {
            onconfigPath: this.formGroup.value.onconfig,
            sqlhostsPath: this.formGroup.value.sqlhost
          }
        });
      }

      observer.next();
      observer.complete();
    });
  }
}
