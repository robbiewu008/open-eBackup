import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DrillDetailRoutingModule } from './drill-detail-routing.module';
import { DrillDetailComponent } from './drill-detail.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ScriptLogModule } from './script-log/script-log.module';

@NgModule({
  declarations: [DrillDetailComponent],
  imports: [
    CommonModule,
    DrillDetailRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ScriptLogModule
  ]
})
export class DrillDetailModule {}
