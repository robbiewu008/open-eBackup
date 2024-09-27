import { NgModule } from '@angular/core';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { BatchOperateServiceModule } from 'app/shared/services/batch-operate.service';
import { BaseModule } from '../../../shared/base.module';
import { AlarmsClearComponent } from './alarms-clear/alarms-clear.component';
import { AlarmsDetailsComponent } from './alarms-details/alarms-details.component';
import { AlarmsRoutingModule } from './alarms-routing.module';
import { AlarmsComponent, SelectionPipe } from './alarms.component';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [
    AlarmsComponent,
    AlarmsDetailsComponent,
    AlarmsClearComponent,
    SelectionPipe
  ],
  imports: [
    BaseModule,
    AlarmsRoutingModule,
    BatchOperateServiceModule,
    MultiClusterSwitchModule,
    CustomTableSearchModule
  ]
})
export class AlarmsModule {}
