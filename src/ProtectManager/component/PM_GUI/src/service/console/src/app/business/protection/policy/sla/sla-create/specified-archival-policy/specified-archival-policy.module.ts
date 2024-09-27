import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { GeneralArchivalPolicyModule } from './general-archival-policy/general-archival-policy.module';
import { SpecifiedArchivalPolicyComponent } from './specified-archival-policy.component';

@NgModule({
  declarations: [SpecifiedArchivalPolicyComponent],
  imports: [CommonModule, BaseModule, GeneralArchivalPolicyModule],

  exports: [SpecifiedArchivalPolicyComponent]
})
export class SpecifiedArchivalPolicyModule {}
