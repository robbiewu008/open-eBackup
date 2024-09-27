import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { SelectDatabaseComponent } from './select-database.component';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';

@NgModule({
  imports: [CommonModule, BaseModule, SelectProtectObjectsModule],
  declarations: [SelectDatabaseComponent],
  exports: [SelectDatabaseComponent]
})
export class SelectDatabaseModule {}
