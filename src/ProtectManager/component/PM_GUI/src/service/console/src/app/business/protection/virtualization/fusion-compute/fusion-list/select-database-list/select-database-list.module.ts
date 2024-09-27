import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddDiskModule } from './add-disk/add-disk.module';
import { SelectDatabaseListComponent } from './select-database-list.component';

@NgModule({
  declarations: [SelectDatabaseListComponent],
  imports: [CommonModule, BaseModule, ProTableModule, AddDiskModule]
})
export class SelectDatabaseListModule {}
