import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { MongodbRoutingModule } from './mongodb-routing.module';
import { MongodbComponent } from './mongodb.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { BaseTemplateModule } from '../../virtualization/kubernetes/base-template/base-template.module';
import { BaseModule } from 'app/shared';
import { RegisterMongodbModule } from './register-mongodb/register-mongodb.module';
import { SummaryModule } from './summary/summary.module';
import { CopyDataModule } from './copy-data/copy-data.module';

@NgModule({
  declarations: [MongodbComponent],
  imports: [
    CommonModule,
    BaseModule,
    MongodbRoutingModule,
    MultiClusterSwitchModule,
    BaseTemplateModule,
    RegisterMongodbModule,
    SummaryModule,
    CopyDataModule
  ]
})
export class MongodbModule {}
