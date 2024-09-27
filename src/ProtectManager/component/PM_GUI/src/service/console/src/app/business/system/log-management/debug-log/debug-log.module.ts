import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { DebugLogRoutingModule } from './debug-log-routing.module';
import { DebugLogComponent } from './debug-log.component';
import { ExportFilesModule } from 'app/shared/components/export-files/export-files.module';
import { MultiLogModule } from '../multi-log/multi-log.module';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [DebugLogComponent],
  imports: [
    CommonModule,
    DebugLogRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ExportFilesModule,
    MultiLogModule,
    AlertModule
  ]
})
export class DebugLogModule {}
