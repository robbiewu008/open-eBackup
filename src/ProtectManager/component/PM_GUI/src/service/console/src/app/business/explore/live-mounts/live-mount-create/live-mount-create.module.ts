import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { LiveMountCreateComponent } from './live-mount-create.component';
import { BaseModule } from 'app/shared';
import { SelectCopyDataModule } from './select-copy-data/select-copy-data.module';
import { SelectResourceModule } from './select-resource/select-resource.module';
import { LiveMountOptionsModule as OracleLiveMountOptionsModule } from '../oracle/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as OracleLiveMountSummaryModule } from '../oracle/live-mount-summary/live-mount-summary.module';
import { LiveMountOptionsModule as VMwareLiveMountOptionsModule } from '../vmware/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as VMwareLiveMountSummaryModule } from '../vmware/live-mount-summary/live-mount-summary.module';
import { LiveMountOptionsModule as NasSharedLiveMountOptionsModule } from '../nas-shared/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as NasSharedLiveMountSummaryModule } from '../nas-shared/live-mount-summary/live-mount-summary.module';
import { LiveMountOptionsModule as FilesetLiveMountOptionsModule } from '../fileset/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as FilesetLiveMountSummaryModule } from '../fileset/live-mount-summary/live-mount-summary.module';
import { LiveMountOptionsModule as CnwareLiveMountOptionsModule } from '../cnware/live-mount-options/live-mount-options.module';
import { LiveMountSummaryModule as CnwareLiveMountSummaryModule } from '../cnware/live-mount-summary/live-mount-summary.module';

@NgModule({
  declarations: [LiveMountCreateComponent],
  imports: [
    CommonModule,
    BaseModule,
    SelectCopyDataModule,
    SelectResourceModule,
    OracleLiveMountOptionsModule,
    OracleLiveMountSummaryModule,
    VMwareLiveMountOptionsModule,
    VMwareLiveMountSummaryModule,
    NasSharedLiveMountOptionsModule,
    NasSharedLiveMountSummaryModule,
    FilesetLiveMountOptionsModule,
    FilesetLiveMountSummaryModule,
    CnwareLiveMountOptionsModule,
    CnwareLiveMountSummaryModule
  ],
  exports: [LiveMountCreateComponent]
})
export class LiveMountCreateModule {}
