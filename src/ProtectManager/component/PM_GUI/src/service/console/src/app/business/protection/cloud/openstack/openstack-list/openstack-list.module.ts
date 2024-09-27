import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { OpenstackListComponent } from './openstack-list.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { SetQuotaModule } from './set-quota/set-quota.module';
import { SummaryModule } from './summary/summary.module';

@NgModule({
  declarations: [OpenstackListComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    SetQuotaModule,
    SummaryModule
  ],
  exports: [OpenstackListComponent]
})
export class OpenstackListModule {}
