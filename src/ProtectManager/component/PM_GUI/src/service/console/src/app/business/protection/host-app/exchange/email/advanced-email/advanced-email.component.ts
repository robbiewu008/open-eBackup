import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormGroup } from '@angular/forms';
import { MessageService } from '@iux/live';
import { BaseUtilService } from 'app/shared';
import { assign, isArray, isEmpty } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced-email',
  templateUrl: './advanced-email.component.html',
  styleUrls: ['./advanced-email.component.less']
})
export class AdvancedEmailComponent implements OnInit {
  resourceData;
  resourceType;
  hostOptions = [];
  formGroup: FormGroup;
  valid$ = new Subject<boolean>();

  constructor(
    public fb: FormBuilder,
    public message: MessageService,
    public baseUtilService: BaseUtilService
  ) {}

  ngOnInit(): void {
    this.initForm();
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.value);
    });
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  initForm() {
    this.formGroup = this.fb.group({
      backup_continue: [true]
    });
    const { protectedObject } = this.resourceData;
    const extParameters = protectedObject?.extParameters || {};
    this.formGroup
      .get('backup_continue')
      .setValue(
        isEmpty(protectedObject) ? true : extParameters?.backup_continue
      );
    this.formGroup.statusChanges.subscribe(() => {
      this.valid$.next(this.formGroup.value);
    });
  }

  onOK() {
    const ext_parameters = {};
    assign(ext_parameters, {
      backup_continue: this.formGroup.value.backup_continue
    });
    return {
      ext_parameters
    };
  }
}
