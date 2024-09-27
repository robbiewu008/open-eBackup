import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RelatedDesensitizationPolicyComponent } from './related-desensitization-policy.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RelatedDesensitizationPolicyComponent],
  imports: [CommonModule, BaseModule],
  exports: [RelatedDesensitizationPolicyComponent]
})
export class RelatedDesensitizationPolicyModule {}
