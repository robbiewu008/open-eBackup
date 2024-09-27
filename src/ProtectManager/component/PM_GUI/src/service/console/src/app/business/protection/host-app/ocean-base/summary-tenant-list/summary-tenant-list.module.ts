import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SummaryTenantListComponent } from './summary-tenant-list.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [SummaryTenantListComponent],
  imports: [CommonModule, BaseModule, ProTableModule]
})
export class SummaryTenantListModule {}
