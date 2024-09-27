import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { CyberAlarmNotifyRoutingModule } from './cyber-alarm-notify-routing.module';
import { CyberAlarmNotifyComponent } from './cyber-alarm-notify.component';
import { BaseModule } from 'app/shared';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddEmailComponent } from './add-email/add-email.component';
import { SysLogNotifyComponent } from './sys-log-notify/sys-log-notify.component';
import { AddServerNodeComponent } from './sys-log-notify/add-server-node/add-server-node.component';

@NgModule({
  declarations: [
    CyberAlarmNotifyComponent,
    AddEmailComponent,
    SysLogNotifyComponent,
    AddServerNodeComponent
  ],
  imports: [
    CommonModule,
    CyberAlarmNotifyRoutingModule,
    BaseModule,
    ProButtonModule,
    ProTableModule
  ],
  exports: [CyberAlarmNotifyComponent]
})
export class CyberAlarmNotifyModule {}
