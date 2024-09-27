import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { BatchOperateServiceModule } from 'app/shared/services/batch-operate.service';
import { LiveMountsListComponent } from './live-mounts-list.component';
import { UpdatePolicyDetailModule } from '../../policy/mount-update-policy/update-policy-detail/update-policy-detail.module';
import { UpdateCopyDataModule } from './update-copy-data/update-copy-data.module';
import { LiveMountSummaryModule as OracleLiveMountSummaryModule } from '../oracle/live-mount-summary/live-mount-summary.module';
import { LiveMountSummaryModule as VMwareLiveMountSummaryModule } from '../oracle/live-mount-summary/live-mount-summary.module';
import { DestroyLiveMountModule } from './destroy-live-mount/destroy-live-mount.module';
import { LiveMountModifyModule } from './live-mount-modify/live-mount-modify.module';
import { LiveMountCreateModule } from '../live-mount-create/live-mount-create.module';
import { LiveMountMigrateModule } from '../vmware/live-mount-migrate/live-mount-migrate.module';
import { LiveMountSummaryModule as NasLiveMountSummaryModule } from '../nas-shared/live-mount-summary/live-mount-summary.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { LiveMountMigrateModule as CnwareLiveMountMigrateModule } from '../cnware/live-mount-migrate/live-mount-migrate.module';
import { LiveMountSummaryModule as CnwareLiveMountSummaryModule } from '../cnware/live-mount-summary/live-mount-summary.module';
import { CustomTableSearchModule } from 'app/shared/components/custom-table-search/custom-table-search.module';

@NgModule({
  declarations: [LiveMountsListComponent],
  imports: [
    CommonModule,
    BaseModule,
    UpdatePolicyDetailModule,
    DestroyLiveMountModule,
    BatchOperateServiceModule,
    OracleLiveMountSummaryModule,
    UpdateCopyDataModule,
    LiveMountCreateModule,
    OracleLiveMountSummaryModule,
    VMwareLiveMountSummaryModule,
    LiveMountModifyModule,
    LiveMountMigrateModule,
    NasLiveMountSummaryModule,
    MultiClusterSwitchModule,
    CnwareLiveMountMigrateModule,
    CnwareLiveMountSummaryModule,
    CustomTableSearchModule
  ],
  exports: [LiveMountsListComponent]
})
export class LiveMountsListModule {}
