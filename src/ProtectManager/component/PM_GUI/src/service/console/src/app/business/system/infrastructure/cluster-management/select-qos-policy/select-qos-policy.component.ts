import { Component, OnInit } from '@angular/core';
import {
  FormGroup,
  FormBuilder,
  FormControl,
  Validators
} from '@angular/forms';
import { I18NService } from 'app/shared';

@Component({
  selector: 'aui-select-qos-policy',
  templateUrl: './select-qos-policy.component.html'
})
export class SelectQosPolicyComponent implements OnInit {
  formGroup: FormGroup;
  qosPolicyOptions;
  requiredLabel = this.i18n.get('common_required_label');
  qosPolicyLabel = this.i18n.get('common_limit_rate_policy_label');
  requiredErrorTip = {
    required: this.requiredLabel
  };
  constructor(public i18n: I18NService, public fb: FormBuilder) {}

  initForm() {
    this.formGroup = this.fb.group({
      qosPolicyType: new FormControl('', Validators.required)
    });
    this.qosPolicyOptions = [
      {
        key: 'qos1',
        label: 'Qos Policy 01',
        isLeaf: true
      }
    ];
  }

  ngOnInit() {
    this.initForm();
  }
}
