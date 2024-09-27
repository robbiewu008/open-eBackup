import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SelectInstanceDatabaseComponent } from './select-instance-database.component';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';

@NgModule({
  declarations: [SelectInstanceDatabaseComponent],
  imports: [CommonModule, BaseModule, SelectProtectObjectsModule],
  exports: [SelectInstanceDatabaseComponent]
})
export class SelectInstanceDatabaseModule {}
