import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { AlarmSettingsComponent } from './alarm-settings.component';
import { AlarmSettingsRoutingModule } from './alarm-settings-routing.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  imports: [
    CommonModule,
    BaseModule,
    AlarmSettingsRoutingModule,
    MultiClusterSwitchModule
  ],
  declarations: [AlarmSettingsComponent]
})
export class AlarmSettingsModule {}
