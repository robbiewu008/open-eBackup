import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AirgapTacticsModule } from 'app/business/explore/policy/airgap/airgap-tactics.module';
import { AntiPolicyModule } from 'app/business/explore/policy/anti-policy-setting/anti-policy/anti-policy.module';
import { MountUpdatePolicyModule } from 'app/business/explore/policy/mount-update-policy/mount-update-policy.module';
import { ReportsListModule } from 'app/business/insight/reports/reports-list/reports-list.module';
import { BaseModule } from 'app/shared';
import { ClientTemplateModule } from '../client-template/client-template.module';
import { NormalResourcesetTemplateModule } from '../normal-resourceset-template/normal-resourceset-template.module';
import { QosTemplateModule } from '../qos-template/qos-template.module';
import { SlaTemplateModule } from '../sla-template/sla-template.module';
import { VirtualCloudTemplateModule } from '../virtual-cloud-template/virtual-cloud-template.module';
import { CreateResourcesetComponent } from './create-resourceset.component';

@NgModule({
  declarations: [CreateResourcesetComponent],
  imports: [
    CommonModule,
    BaseModule,
    NormalResourcesetTemplateModule,
    VirtualCloudTemplateModule,
    ClientTemplateModule,
    SlaTemplateModule,
    QosTemplateModule,
    AirgapTacticsModule,
    AntiPolicyModule,
    ReportsListModule,
    MountUpdatePolicyModule
  ],
  exports: [CreateResourcesetComponent]
})
export class CreateResourcesetModule {}
