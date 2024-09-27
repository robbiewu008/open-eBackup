import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DatabaseTemplateModule } from '../../database-template/database-template.module';
import { DatabaseComponent } from './database.component';

@NgModule({
  declarations: [DatabaseComponent],
  imports: [CommonModule, BaseModule, DatabaseTemplateModule],
  exports: [DatabaseComponent]
})
export class DatabaseModule {}
