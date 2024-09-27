import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DesensitizationPolicyCardComponent } from './desensitization-policy-card.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [DesensitizationPolicyCardComponent],
  imports: [CommonModule, BaseModule],
  exports: [DesensitizationPolicyCardComponent]
})
export class DesensitizationPolicyCardModule {}
