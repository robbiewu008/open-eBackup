import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { Subject } from 'rxjs';
import { assign, isArray, isEmpty } from 'lodash';

@Component({
  selector: 'aui-advanced-parameter',
  templateUrl: './advanced-parameter.component.html',
  styleUrls: ['./advanced-parameter.component.less']
})
export class AdvancedParameterComponent implements OnInit {
  formGroup: FormGroup;
  resourceData;
  resourceType;
  valid$ = new Subject<boolean>();
  constructor(private fb: FormBuilder) {}

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
      object_backup: new FormControl(true)
    });
    const { protectedObject } = this.resourceData;
    const extParameters = protectedObject?.extParameters || {};
    this.formGroup
      .get('object_backup')
      .setValue(isEmpty(protectedObject) ? true : extParameters?.object_backup);
  }

  updateData() {}

  onOK() {
    const ext_parameters = {};
    assign(ext_parameters, {
      object_backup: this.formGroup.value.object_backup
    });

    return {
      ext_parameters
    };
  }
}
