import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateDesensitizationPolicyComponent } from './create-desensitization-policy.component';
import { BaseModule } from 'app/shared';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [CreateDesensitizationPolicyComponent],
  imports: [CommonModule, BaseModule, CustomTableSearchModule],
  exports: [CreateDesensitizationPolicyComponent]
})
export class CreateDesensitizationPolicyModule {}
