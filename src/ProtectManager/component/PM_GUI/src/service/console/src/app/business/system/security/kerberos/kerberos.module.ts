import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { CreateKerberosModule } from './create-kerberos/create-kerberos.module';
import { KerberosRoutingModule } from './kerberos-routing.module';
import { KerberosComponent } from './kerberos.component';

@NgModule({
  declarations: [KerberosComponent],
  imports: [
    CommonModule,
    KerberosRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    CreateKerberosModule,
    MultiClusterSwitchModule
  ]
})
export class KerberosModule {}
