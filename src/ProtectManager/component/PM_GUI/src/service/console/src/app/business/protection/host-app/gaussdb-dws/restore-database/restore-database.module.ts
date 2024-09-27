import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { DatabaseRestoreComponent } from './restore-database.component';

@NgModule({
  declarations: [DatabaseRestoreComponent],
  imports: [CommonModule, BaseModule],
  exports: [DatabaseRestoreComponent]
})
export class DatabaseRestoreModule {}
