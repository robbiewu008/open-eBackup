import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CloudBackupOverviewComponent } from './cloud-backup-overview.component';
import { CloudBackupOverviewRoutingModule } from './cloud-backup-overview-routing.module';
import { BaseModule } from 'app/shared';
import { DetectionStatisticsModule } from '../detection-statistics/detection-statistics.module';
import { DetectionAlarmModule } from '../detection-alarm/detection-alarm.module';
import { SnapshotDetectionListModule } from '../snapshot-detection-list/snapshot-detection-list.module';

@NgModule({
  declarations: [CloudBackupOverviewComponent],
  imports: [
    CommonModule,
    BaseModule,
    DetectionStatisticsModule,
    CloudBackupOverviewRoutingModule,
    SnapshotDetectionListModule,
    DetectionAlarmModule
  ]
})
export class CloudBackupOverviewModule {}
