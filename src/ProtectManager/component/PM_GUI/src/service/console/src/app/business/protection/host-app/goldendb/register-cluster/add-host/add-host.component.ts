import { ModalRef } from '@iux/live';
import { FormGroup, FormBuilder, FormControl } from '@angular/forms';
import { Component, OnInit } from '@angular/core';
import { Observable, Observer } from 'rxjs';
import { I18NService, BaseUtilService } from 'app/shared';
import { filter, find, get, includes, isUndefined } from 'lodash';
import { USER_GUIDE_CACHE_DATA } from 'app/shared/consts/guide-config';

@Component({
  selector: 'aui-add-host',
  templateUrl: './add-host.component.html',
  styleUrls: ['./add-host.component.less']
})
export class AddHostComponent implements OnInit {
  rowData;
  name;
  data;
  children = [];
  isTest = false;
  okLoading = false;
  testLoading = false;
  options = [];
  hostOptions = [];
  formGroup: FormGroup;

  usernameErrorTip = {
    ...this.baseUtilService.requiredErrorTip,
    ...this.baseUtilService.lengthErrorTip,
    invalidMaxLength: this.i18n.get('common_valid_maxlength_label', [32])
  };

  constructor(
    private fb: FormBuilder,
    public modal: ModalRef,
    private i18n: I18NService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit() {
    this.initForm();
    this.initData();
  }

  showGuideNew(item): boolean {
    return (
      USER_GUIDE_CACHE_DATA.active &&
      includes(USER_GUIDE_CACHE_DATA.host, item.endpoint)
    );
  }

  initForm() {
    this.formGroup = this.fb.group({
      parentUuid: new FormControl('', {
        validators: [this.baseUtilService.VALID.required()]
      }),
      osUser: new FormControl('', {
        validators: [
          this.baseUtilService.VALID.required(),
          this.baseUtilService.VALID.maxLength(32)
        ]
      })
    });
  }

  initData() {
    this.hostOptions = filter(
      this.options,
      item => !find(this.children, val => val.parentUuid === item.uuid)
    );

    if (this.rowData) {
      const originalHost = find(this.options, {
        uuid: this.rowData.parentUuid
      });

      if (isUndefined(originalHost)) {
        return;
      }
      this.hostOptions = [originalHost, ...this.hostOptions];
      this.formGroup.get('osUser').setValue(this.rowData.osUser);
      this.formGroup.get('parentUuid').setValue(get(originalHost, 'uuid'));
    }
  }

  onOK(): Observable<void> {
    return new Observable<void>((observer: Observer<void>) => {
      if (this.formGroup.invalid) {
        return;
      }

      const host = find(this.hostOptions, {
        uuid: this.formGroup.value.parentUuid
      });

      this.data = {
        parentName: `${get(host, 'name')}(${get(host, 'endpoint')})`,
        parentUuid: this.formGroup.value.parentUuid,
        osUser: this.formGroup.value.osUser
      };
      observer.next();
      observer.complete();
    });
  }
}
