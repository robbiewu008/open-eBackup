import { Component, OnInit, ViewChild } from '@angular/core';
import { DesensitizationPolicyListComponent } from './desensitization-policy-list/desensitization-policy-list.component';
import { IdentifiedRuleComponent } from './identified-rule/identified-rule.component';
import { DesensitizationRuleComponent } from './desensitization-rule/desensitization-rule.component';

@Component({
  selector: 'aui-desensitization-policy',
  templateUrl: './desensitization-policy.component.html',
  styleUrls: ['./desensitization-policy.component.less']
})
export class DesensitizationPolicyComponent implements OnInit {
  activeIndex = 0;

  @ViewChild(DesensitizationPolicyListComponent, { static: false })
  desensitizationPolicyListComponent: DesensitizationPolicyListComponent;

  @ViewChild(IdentifiedRuleComponent, { static: false })
  identifiedRuleComponent: IdentifiedRuleComponent;

  @ViewChild(DesensitizationRuleComponent, { static: false })
  desensitizationRuleComponent: DesensitizationRuleComponent;

  constructor() {}

  ngOnInit() {}

  onChange() {
    if (this.activeIndex === 0) {
      this.desensitizationPolicyListComponent.policyData = [];
      this.desensitizationPolicyListComponent.total = 0;
      this.desensitizationPolicyListComponent.ngOnInit();
    } else if (this.activeIndex === 1) {
      this.identifiedRuleComponent.tableData = [];
      this.identifiedRuleComponent.total = 0;
      this.identifiedRuleComponent.selection = [];
      this.identifiedRuleComponent.ngOnInit();
    } else {
      this.desensitizationRuleComponent.tableData = [];
      this.desensitizationRuleComponent.total = 0;
      this.desensitizationRuleComponent.selection = [];
      this.desensitizationRuleComponent.ngOnInit();
    }
  }
}
