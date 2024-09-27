import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { SummaryTenantComponent } from './summary-tenant.component';
import { BaseInfoModule } from 'app/shared/components/base-info/base-info.module';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProStatusModule } from 'app/shared/components/pro-status';

@NgModule({
  declarations: [SummaryTenantComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseInfoModule,
    ProTableModule,
    ProStatusModule
  ]
})
export class SummaryTenantModule {}
