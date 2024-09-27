import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { AlertModule, TransferModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { StartSftpComponent } from './start-sftp.component';

@NgModule({
  declarations: [StartSftpComponent],
  imports: [CommonModule, BaseModule, TransferModule, AlertModule],

  exports: [StartSftpComponent]
})
export class StartSftpModule {}
