import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { ExportFilesModule } from '../export-files/export-files.module';
import { DownloadFlrFilesComponent } from './download-flr-files.component';

@NgModule({
  declarations: [DownloadFlrFilesComponent],
  imports: [CommonModule, BaseModule, ExportFilesModule],
  exports: [DownloadFlrFilesComponent]
})
export class DownloadFlrFilesModule {}
