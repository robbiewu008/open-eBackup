import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DesensitizationPolicyDetailComponent } from './desensitization-policy-detail.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [DesensitizationPolicyDetailComponent],
  imports: [CommonModule, BaseModule],
  exports: [DesensitizationPolicyDetailComponent]
})
export class DesensitizationPolicyDetailModule {}
