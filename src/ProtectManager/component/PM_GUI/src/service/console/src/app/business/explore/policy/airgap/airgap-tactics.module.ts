import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { CreateAirgapModule } from './create-airgap/create-airgap.module';
import { AirgapDetailModule } from './airgap-detail/airgap-detail.module';
import { AirgapTactisComponent } from './airgap-tactics.component';

@NgModule({
  declarations: [AirgapTactisComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    MultiClusterSwitchModule,
    CreateAirgapModule,
    AirgapDetailModule
  ],
  exports: [AirgapTactisComponent]
})
export class AirgapTacticsModule {}
