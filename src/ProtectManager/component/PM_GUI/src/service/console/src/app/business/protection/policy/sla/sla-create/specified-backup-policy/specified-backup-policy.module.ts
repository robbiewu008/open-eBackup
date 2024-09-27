import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AdvancedParametersModule } from './advanced-parameters/advanced-parameters.module';
import { BackupPolicyModule } from './backup-policy/backup-policy.module';
import { GeneralAdvancedParameterModule } from './general-advanced-parameter/general-advanced-parameter.module';
import { HyperVParameterModule } from './hyper-v-parameter/hyper-v-parameter.module';
import { LocalFileSystemAdvancedParameterModule } from './local-file-system-advanced-parameter/local-file-system-advanced-parameter.module';
import { SpecifiedBackupPolicyComponent } from './specified-backup-policy.component';
import { VmwareAdvancedParameterModule } from './vmware-advanced-parameter/vmware-advanced-parameter.module';

@NgModule({
  declarations: [SpecifiedBackupPolicyComponent],
  imports: [
    CommonModule,
    BaseModule,
    BackupPolicyModule,
    HyperVParameterModule,
    VmwareAdvancedParameterModule,
    LocalFileSystemAdvancedParameterModule,
    GeneralAdvancedParameterModule,
    AdvancedParametersModule
  ],
  exports: [SpecifiedBackupPolicyComponent]
})
export class SpecifiedBackupPolicyModule {}
