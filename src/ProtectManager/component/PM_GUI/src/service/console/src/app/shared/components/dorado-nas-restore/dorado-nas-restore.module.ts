import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared/base.module';
import { DoradoNasRestoreComponent } from './dorado-nas-restore.component';
import { CreateFileSystemModule } from './create-file-system/create-file-system.module';
import { CreateKerberosModule } from 'app/business/system/security/kerberos/create-kerberos/create-kerberos.module';
import { RegisterNasShareModule } from 'app/business/protection/storage/nas-shared/register-nas-share/register-nas-share.module';

@NgModule({
  declarations: [DoradoNasRestoreComponent],
  imports: [
    CommonModule,
    BaseModule,
    CreateFileSystemModule,
    CreateKerberosModule,
    RegisterNasShareModule
  ],
  exports: [DoradoNasRestoreComponent]
})
export class DoradoNasRestoreModule {}
