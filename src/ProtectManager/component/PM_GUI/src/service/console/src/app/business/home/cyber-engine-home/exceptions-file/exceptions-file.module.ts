import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { ExceptionsFileComponent } from './exceptions-file.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  imports: [CommonModule, BaseModule, ProTableModule],
  declarations: [ExceptionsFileComponent],
  exports: [ExceptionsFileComponent]
})
export class ExceptionsFileModule {}
