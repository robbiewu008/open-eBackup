import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FileSystemComponent } from './file-system.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { FileExtensionDetailModule } from './file-extension-detail/file-extension-detail.module';

@NgModule({
  declarations: [FileSystemComponent],
  imports: [
    CommonModule,
    BaseModule,
    ProTableModule,
    ProButtonModule,
    FileExtensionDetailModule
  ],
  exports: [FileSystemComponent]
})
export class FileSystemModule {}
