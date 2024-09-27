import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { JobResourceModule } from 'app/business/insight/job/job-resource/job-resource.module';
import { BaseModule } from 'app/shared';
import { ModifyRetentionPolicyModule } from 'app/shared/components';
import { DetailModalModule } from 'app/shared/components/detail-modal/detail-modal.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProStatusModule } from 'app/shared/components/pro-status';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProtectModule } from 'app/shared/components/protect/protect.module';
import { SlaServiceModule } from 'app/shared/services/sla.service';
import { KingBaseInstanceDatabaseComponent } from './king-base-instance-database.component';
import { KingBaseRegisterModule } from './register/king-base-register.module';
import { KingBaseSummaryModule } from './summary/king-base-summary.module';

@NgModule({
  declarations: [KingBaseInstanceDatabaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    ProStatusModule,
    ProtectModule,
    JobResourceModule,
    DetailModalModule,
    SlaServiceModule,
    ModifyRetentionPolicyModule,
    KingBaseRegisterModule,
    KingBaseSummaryModule
  ],
  exports: [KingBaseInstanceDatabaseComponent]
})
export class KingBaseInstanceDatabaseModule {}
