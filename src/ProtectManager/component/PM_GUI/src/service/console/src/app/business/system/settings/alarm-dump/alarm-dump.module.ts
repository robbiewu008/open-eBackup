import { NgModule } from '@angular/core';
import { AlarmDumpRoutingModule } from './alarm-dump-routing.module';
import { AlarmDumpComponent } from './alarm-dump.component';
import { BaseModule } from 'app/shared/base.module';
import { DumpSftpModule } from './dump-sftp/dump-sftp.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [AlarmDumpComponent],
  imports: [
    AlarmDumpRoutingModule,
    BaseModule,
    DumpSftpModule,
    MultiClusterSwitchModule
  ]
})
export class AlarmDumpModule {}
