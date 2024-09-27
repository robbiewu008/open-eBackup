import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FileSystemNumComponent } from './file-system-num.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [FileSystemNumComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [FileSystemNumComponent]
})
export class FileSystemNumModule {}
