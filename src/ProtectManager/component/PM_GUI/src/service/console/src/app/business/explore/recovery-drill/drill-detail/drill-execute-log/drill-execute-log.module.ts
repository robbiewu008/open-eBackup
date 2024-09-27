import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { DrillExecuteLogRoutingModule } from './drill-execute-log-routing.module';
import { DrillExecuteLogComponent } from './drill-execute-log.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ScriptLogModule } from '../script-log/script-log.module';

@NgModule({
  declarations: [DrillExecuteLogComponent],
  imports: [
    CommonModule,
    DrillExecuteLogRoutingModule,
    BaseModule,
    ProTableModule,
    ScriptLogModule
  ]
})
export class DrillExecuteLogModule {}
0;
