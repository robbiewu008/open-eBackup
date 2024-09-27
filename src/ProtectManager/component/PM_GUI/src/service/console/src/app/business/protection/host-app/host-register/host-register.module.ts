import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AlertModule } from '@iux/live';

import { HostRegisterRoutingModule } from './host-register-routing.module';
import { HostRegisterComponent } from './host-register.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddHostIngfoComponent } from './add-host-ingfo/add-host-ingfo.component';

@NgModule({
  declarations: [HostRegisterComponent, AddHostIngfoComponent],
  imports: [
    CommonModule,
    HostRegisterRoutingModule,
    BaseModule,
    ProTableModule,
    AlertModule
  ]
})
export class HostRegisterModule {}
