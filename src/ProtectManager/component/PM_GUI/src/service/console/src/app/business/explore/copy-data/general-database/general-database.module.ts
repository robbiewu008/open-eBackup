import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyResourceListModule } from '../copy-resource-list/copy-resource-list.module';
import { GeneralDatabaseRoutingModule } from './general-database-routing.module';
import { GeneralDatabaseComponent } from './general-database.component';

@NgModule({
  declarations: [GeneralDatabaseComponent],
  imports: [
    CommonModule,
    GeneralDatabaseRoutingModule,
    BaseModule,
    CopyResourceListModule
  ]
})
export class GeneralDatabaseModule {}
