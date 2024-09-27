import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';
import { UserQuotaRoutingModule } from './user-quota-routing.module';
import { UserQuotaComponent } from './user-quota.component';
import { SetQuotaModule } from './set-quota/set-quota.module';

@NgModule({
  declarations: [UserQuotaComponent],
  imports: [
    CommonModule,
    UserQuotaRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    MultiClusterSwitchModule,
    SetQuotaModule
  ]
})
export class UserQuotaModule {}
