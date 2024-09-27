import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { CopyDataModule } from '../database-template/copy-data/copy-data.module';
import { OceanBaseRoutingModule } from './ocean-base-routing.module';
import { OceanBaseComponent } from './ocean-base.component';
import { RegisterClusterModule } from './register-cluster/register-cluster.module';
import { RegisterTenantModule } from './register-tenant/register-tenant.module';
import { SummaryClusterModule } from './summary-cluster/summary-cluster.module';
import { SummaryTenantListModule } from './summary-tenant-list/summary-tenant-list.module';
import { SummaryTenantModule } from './summary-tenant/summary-tenant.module';
@NgModule({
  declarations: [OceanBaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    DatabaseTemplateModule,
    OceanBaseRoutingModule,
    RegisterClusterModule,
    RegisterTenantModule,
    SummaryClusterModule,
    SummaryTenantListModule,
    SummaryTenantModule,
    CopyDataModule
  ]
})
export class OceanBaseModule {}
