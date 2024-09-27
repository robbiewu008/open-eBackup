import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { DatabaseComponent } from './database.component';
import { BaseModule } from 'app/shared';
import { BaseTemplateModule } from '../base-template/base-template.module';
import { ProButtonModule } from 'app/shared/components/pro-button';
import { ProTableModule } from 'app/shared/components/pro-table';

@NgModule({
  declarations: [DatabaseComponent],
  imports: [
    CommonModule,
    BaseModule,
    BaseTemplateModule,
    ProButtonModule,
    ProTableModule
  ],
  exports: [DatabaseComponent]
})
export class DatabaseModule {}
