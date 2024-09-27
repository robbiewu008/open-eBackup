import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import {
  DesensitizationPolicyListComponent,
  SelectionPipe
} from './desensitization-policy-list.component';
import { DesensitizationPolicyCardModule } from '../desensitization-policy-card/desensitization-policy-card.module';
import { CreateDesensitizationPolicyModule } from './create-desensitization-policy/create-desensitization-policy.module';
import { DesensitizationPolicyDetailModule } from './desensitization-policy-detail/desensitization-policy-detail.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [DesensitizationPolicyListComponent, SelectionPipe],
  imports: [
    CommonModule,
    BaseModule,
    DesensitizationPolicyCardModule,
    CreateDesensitizationPolicyModule,
    DesensitizationPolicyDetailModule,
    CustomTableSearchModule
  ],
  exports: [DesensitizationPolicyListComponent]
})
export class DesensitizationPolicyListModule {}
