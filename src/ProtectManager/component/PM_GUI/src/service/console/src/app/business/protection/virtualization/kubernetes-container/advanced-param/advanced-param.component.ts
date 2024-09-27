import { Component, OnInit } from '@angular/core';
import { FormBuilder, FormControl, FormGroup } from '@angular/forms';
import { DataMap, I18NService } from 'app/shared';
import { AppUtilsService } from 'app/shared/services/app-utils.service';
import { isArray, isString } from 'lodash';
import { Subject } from 'rxjs';

@Component({
  selector: 'aui-advanced-param',
  templateUrl: './advanced-param.component.html',
  styleUrls: ['./advanced-param.component.less']
})
export class AdvancedParamComponent implements OnInit {
  resourceData;
  resourceType;
  formGroup: FormGroup;
  dataMap = DataMap;

  backupHelp = this.i18n.get('protetion_kubernetes_advanced_help_label');

  valid$ = new Subject<boolean>();

  constructor(
    private fb: FormBuilder,
    private i18n: I18NService,
    private appUtilsService: AppUtilsService
  ) {}

  ngOnInit() {
    this.initForm();
    this.updateForm();
  }

  backupHelpHover() {
    const url = this.i18n.isEn
      ? '/console/assets/help/a8000/en-us/index.html#en-us_topic_0000001792381352.html'
      : '/console/assets/help/a8000/zh-cn/index.html#zh-cn_topic_0000001792381352.html';
    this.appUtilsService.openSpecialHelp(url);
  }

  initData(data: any, resourceType: string) {
    this.resourceData = isArray(data) ? data[0] : data;
    this.resourceType = resourceType;
  }

  initForm() {
    this.formGroup = this.fb.group({
      is_consistent: new FormControl(false)
    });
    this.listenForm();
  }

  listenForm() {
    this.formGroup.statusChanges.subscribe(() =>
      this.valid$.next(this.formGroup.valid)
    );
  }

  updateForm() {
    if (!this.resourceData.protectedObject?.extParameters) {
      return;
    }
    const extParameters = isString(
      this.resourceData.protectedObject?.extParameters
    )
      ? JSON.parse(this.resourceData.protectedObject?.extParameters)
      : this.resourceData.protectedObject?.extParameters;
    this.formGroup.patchValue({
      is_consistent: extParameters.is_consistent
    });
  }

  onOK() {
    const ext_parameters = {
      is_consistent: this.formGroup.value.is_consistent
    };

    return {
      ext_parameters
    };
  }
}
