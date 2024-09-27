import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';

import { RealTimeDetectionRoutingModule } from './real-time-detection-routing.module';
import { RealTimeDetectionComponent } from './real-time-detection.component';
import { FileSystemComponent } from './file-system/file-system.component';
import { DetectionPolicyComponent } from './detection-policy/detection-policy.component';
import { WhiteListComponent } from './white-list/white-list.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { CreateWhiteListComponent } from './white-list/create-white-list/create-white-list.component';
import { AssociatePolicyComponent } from './white-list/associate-policy/associate-policy.component';
import { PolicyDetailComponent } from './detection-policy/policy-detail/policy-detail.component';
import { AssociatedFileSystemComponent } from './detection-policy/associated-file-system/associated-file-system.component';
import { AssociatedWhiteListComponent } from './detection-policy/associated-white-list/associated-white-list.component';
import { WarnModalComponent } from './file-system/warn-modal/warn-modal.component';
import { HoneypotDetailComponent } from './file-system/honeypot-detail/honeypot-detail.component';

@NgModule({
  declarations: [
    RealTimeDetectionComponent,
    FileSystemComponent,
    DetectionPolicyComponent,
    WhiteListComponent,
    CreateWhiteListComponent,
    AssociatePolicyComponent,
    PolicyDetailComponent,
    AssociatedFileSystemComponent,
    AssociatedWhiteListComponent,
    WarnModalComponent,
    HoneypotDetailComponent
  ],
  imports: [
    CommonModule,
    RealTimeDetectionRoutingModule,
    BaseModule,
    ProTableModule,
    ProButtonModule
  ]
})
export class RealTimeDetectionModule {}
