import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { RecoveryDrillRoutingModule } from './recovery-drill-routing.module';
import { RecoveryDrillComponent } from './recovery-drill.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';

@NgModule({
  declarations: [RecoveryDrillComponent],
  imports: [
    CommonModule,
    RecoveryDrillRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule
  ]
})
export class RecoveryDrillModule {}
