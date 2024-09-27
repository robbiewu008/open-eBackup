import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { LocalFileSystemRestoreComponent } from './local-file-system-restore.component';

@NgModule({
  declarations: [LocalFileSystemRestoreComponent],
  imports: [CommonModule, BaseModule]
})
export class LocalFileSystemRestoreModule {}
