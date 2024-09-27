import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ExportFilesComponent } from './export-files.component';
import { AlertModule } from '@iux/live';

@NgModule({
  declarations: [ExportFilesComponent],
  imports: [CommonModule, BaseModule, AlertModule]
})
export class ExportFilesModule {}
