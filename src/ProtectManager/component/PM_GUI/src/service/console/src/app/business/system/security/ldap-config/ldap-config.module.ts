import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { LdapConfigRoutingModule } from './ldap-config-routing.module';
import { BaseModule } from 'app/shared';
import { LdapConfigComponent } from './ldap-config.component';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [LdapConfigComponent],
  imports: [
    CommonModule,
    LdapConfigRoutingModule,
    BaseModule,
    MultiClusterSwitchModule
  ]
})
export class LdapConfigModule {}
