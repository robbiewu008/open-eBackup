import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DumpSftpComponent } from './dump-sftp.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [DumpSftpComponent],
  imports: [CommonModule, BaseModule],
  exports: [DumpSftpComponent]
})
export class DumpSftpModule {}
