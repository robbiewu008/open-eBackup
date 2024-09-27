import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { FileDetailModule } from './file-detail/file-detail.module';
import { FileListComponent } from './file-list.component';

@NgModule({
  declarations: [FileListComponent],
  imports: [CommonModule, BaseModule, FileDetailModule],

  exports: [FileListComponent]
})
export class FileListModule {}
