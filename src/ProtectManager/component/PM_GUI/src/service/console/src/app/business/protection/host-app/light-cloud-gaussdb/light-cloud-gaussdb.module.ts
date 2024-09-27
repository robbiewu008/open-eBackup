import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { DatabaseTemplateModule } from '../database-template/database-template.module';
import { LightCloudGaussdbRoutingModule } from './light-cloud-gaussdb-routing.module';
import { LightCloudGaussdbComponent } from './light-cloud-gaussdb.component';
import { SummaryModule } from './summary/summary.module';
import { SummaryModule as ProjectSummaryModule } from './project-summary/summary.module';
import { RegisterModule } from './register/register.module';

@NgModule({
  declarations: [LightCloudGaussdbComponent],
  imports: [
    CommonModule,
    LightCloudGaussdbRoutingModule,
    BaseModule,
    DatabaseTemplateModule,
    SummaryModule,
    ProjectSummaryModule,
    MultiClusterSwitchModule,
    RegisterModule
  ]
})
export class LightCloudGaussdbModule {}
