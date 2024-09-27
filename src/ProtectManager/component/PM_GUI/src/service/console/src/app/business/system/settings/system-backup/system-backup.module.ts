import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { PolicyConfigModule } from './policy-config/policy-config.module';
import { SystemBackupRoutingModule } from './system-backup-routing.module';
import { SystemBackupComponent } from './system-backup.component';
import { ManuallBackupModule } from './manuall-backup/manuall-backup.module';
import { BackupRestoreModule } from './backup-restore/backup-restore.module';
import { ImportBackupModule } from './import-backup/import-backup.module';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';

@NgModule({
  declarations: [SystemBackupComponent],
  imports: [
    BaseModule,
    SystemBackupRoutingModule,
    ManuallBackupModule,
    PolicyConfigModule,
    BackupRestoreModule,
    ImportBackupModule,
    MultiClusterSwitchModule
  ]
})
export class SystemBackupModule {}
