import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { FileExportComponent } from './file-export.component';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [FileExportComponent],
  imports: [CommonModule, BaseModule, AlertModule],
  exports: [FileExportComponent]
})
export class FileExportModule {}
