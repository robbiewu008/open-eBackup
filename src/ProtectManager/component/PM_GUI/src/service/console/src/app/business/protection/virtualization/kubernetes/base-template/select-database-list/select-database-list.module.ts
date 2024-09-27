import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SelectDatabaseListComponent } from './select-database-list.component';
import { BaseModule } from 'app/shared';
import { ProTableModule } from 'app/shared/components/pro-table';
import { AddVolumeModule } from './add-volume/add-volume.module';

@NgModule({
  declarations: [SelectDatabaseListComponent],
  imports: [CommonModule, BaseModule, ProTableModule, AddVolumeModule]
})
export class SelectDatabaseListModule {}
