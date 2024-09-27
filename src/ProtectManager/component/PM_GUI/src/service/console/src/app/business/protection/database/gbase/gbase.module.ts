import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { GbaseRoutingModule } from './gbase-routing.module';
import { GbaseComponent } from './gbase.component';
import { InstanceDatabaseModule } from '../../host-app/gaussdb-dws/instance-database/instance-database.module';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [GbaseComponent],
  imports: [
    CommonModule,
    GbaseRoutingModule,
    BaseModule,
    InstanceDatabaseModule
  ]
})
export class GbaseModule {}
