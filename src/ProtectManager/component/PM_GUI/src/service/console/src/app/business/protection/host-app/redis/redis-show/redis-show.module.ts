import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { RedisShowComponent } from './redis-show.component';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { ModifyRetentionPolicyModule } from 'app/shared/components';
import { RedisShowRoutingModule } from './redis-show-routing.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { RedisCopyDataModule } from '../copy-data/redis-copy-data.module';

@NgModule({
  declarations: [RedisShowComponent],
  imports: [
    CommonModule,
    RedisShowRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    ProtectModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    ModifyRetentionPolicyModule,
    MultiClusterSwitchModule,
    RedisCopyDataModule
  ],
  exports: [RedisShowComponent]
})
export class RedisShowModule {}
