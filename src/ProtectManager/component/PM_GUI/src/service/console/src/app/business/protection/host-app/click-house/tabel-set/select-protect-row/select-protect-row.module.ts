import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { SelectProtectRowComponent } from './select-protect-row.component';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';

@NgModule({
  imports: [CommonModule, BaseModule, SelectProtectObjectsModule],
  declarations: [SelectProtectRowComponent],
  exports: [SelectProtectRowComponent]
})
export class SelectProtectRowModule {}
