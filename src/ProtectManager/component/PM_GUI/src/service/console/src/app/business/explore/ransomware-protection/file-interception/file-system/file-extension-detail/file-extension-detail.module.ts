import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { FileExtensionDetailComponent } from './file-extension-detail.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [FileExtensionDetailComponent],
  imports: [CommonModule, BaseModule, ProTableModule],
  exports: [FileExtensionDetailComponent]
})
export class FileExtensionDetailModule {}
