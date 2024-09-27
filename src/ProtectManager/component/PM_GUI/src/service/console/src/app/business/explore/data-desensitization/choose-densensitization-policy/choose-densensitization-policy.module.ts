import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ChooseDensensitizationPolicyComponent } from './choose-densensitization-policy.component';
import { DesensitizationPolicyCardModule } from '../../policy/desensitization-policy/desensitization-policy-card/desensitization-policy-card.module';
import { CreateDesensitizationPolicyModule } from '../../policy/desensitization-policy/desensitization-policy-list/create-desensitization-policy/create-desensitization-policy.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [ChooseDensensitizationPolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    DesensitizationPolicyCardModule,
    CreateDesensitizationPolicyModule,
    CustomTableSearchModule
  ],
  exports: [ChooseDensensitizationPolicyComponent]
})
export class ChooseDensensitizationPolicyModule {}
