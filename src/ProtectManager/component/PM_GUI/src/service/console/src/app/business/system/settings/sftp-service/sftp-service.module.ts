import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { MultiClusterSwitchModule } from 'app/shared/components/multi-cluster-switch/multi-cluster-switch.module';
import { ProcessLoadingModule } from 'app/shared/components/process-loading/process-loading.module';
import { AddUserModule } from './add-user/add-user.module';
import { ChangePasswordModule } from './change-password/change-password.module';
import { SftpServiceRoutingModule } from './sftp-service-routing.module';
import { SftpServiceComponent } from './sftp-service.component';
import { StartSftpModule } from './start-sftp/start-sftp.module';
import { ThresholdModifyModule } from './threshold-modify/threshold-modify.module';
import { UserDetailModule } from './user-detail/user-detail.module';

@NgModule({
  declarations: [SftpServiceComponent],
  imports: [
    CommonModule,
    AddUserModule,
    UserDetailModule,
    ChangePasswordModule,
    ProcessLoadingModule,
    SftpServiceRoutingModule,
    BaseModule,
    MultiClusterSwitchModule,
    ThresholdModifyModule,
    StartSftpModule
  ]
})
export class SftpServiceModule {}
