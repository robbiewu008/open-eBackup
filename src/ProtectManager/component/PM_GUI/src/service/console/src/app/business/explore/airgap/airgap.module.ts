import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AirgapRoutingModule } from './airgap-routing.modules';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { AirgapComponent } from './airgap.component';
import { AirgapTacticsModule } from '../policy/airgap/airgap-tactics.module';
import { StorageDeviceComponent } from './storage-device/storage-device.component';
import { StorageDeviceModule } from './storage-device/storage-device.module';

@NgModule({
  declarations: [AirgapComponent],
  imports: [
    CommonModule,
    BaseModule,
    AirgapRoutingModule,
    MultiClusterSwitchModule,
    AirgapTacticsModule,
    StorageDeviceModule
  ]
})
export class AirgapModule {}
