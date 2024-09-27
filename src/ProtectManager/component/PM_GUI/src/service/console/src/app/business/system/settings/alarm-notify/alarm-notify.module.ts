import { NgModule } from '@angular/core';
import { AlarmNotifyRoutingModule } from './alarm-notify-routing.module';
import { AlarmNotifyComponent } from './alarm-notify.component';
import { BaseModule } from 'app/shared/base.module';
import { AlarmsModule } from 'app/business/insight/alarms/alarms.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [AlarmNotifyComponent],
  imports: [
    AlarmNotifyRoutingModule,
    BaseModule,
    AlarmsModule,
    MultiClusterSwitchModule
  ]
})
export class AlarmNotifyModule {}
