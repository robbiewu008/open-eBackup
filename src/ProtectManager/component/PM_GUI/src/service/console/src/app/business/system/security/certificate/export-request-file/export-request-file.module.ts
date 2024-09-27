import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ExportRequestFileComponent } from './export-request-file.component';

@NgModule({
  declarations: [ExportRequestFileComponent],
  imports: [CommonModule, BaseModule],
  exports: [ExportRequestFileComponent]
})
export class ExportRequestFileModule {}
