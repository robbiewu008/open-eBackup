import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { KingBaseClusterModule } from './cluster/king-base-cluster.module';
import { KingBaseCopyDataModule } from './instance-database/copy-data/king-base-copy-data.module';
import { KingBaseInstanceDatabaseModule } from './instance-database/king-base-instance-database.module';
import { KingBaseRoutingModule } from './king-base-routing.module';
import { KingBaseComponent } from './king-base.component';

@NgModule({
  declarations: [KingBaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    KingBaseClusterModule,
    KingBaseRoutingModule,
    KingBaseInstanceDatabaseModule,
    MultiClusterSwitchModule,
    KingBaseCopyDataModule
  ]
})
export class KingBaseModule {}
