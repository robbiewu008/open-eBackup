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
import { TableTemplateComponent } from './table-template.component';
import { InstanceRegisterModule } from '../instance-register/instance-register.module';

@NgModule({
  declarations: [TableTemplateComponent],
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
    InstanceRegisterModule,
    ModifyRetentionPolicyModule
  ],
  exports: [TableTemplateComponent]
})
export class TableTemplateModule {}
