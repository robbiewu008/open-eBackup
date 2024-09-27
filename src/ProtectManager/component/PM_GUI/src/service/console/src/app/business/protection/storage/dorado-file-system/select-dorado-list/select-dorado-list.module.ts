import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';
import { SelectDoradoListComponent } from './select-dorado-list.component';

@NgModule({
  declarations: [SelectDoradoListComponent],
  imports: [CommonModule, BaseModule, SelectProtectObjectsModule],
  exports: [SelectDoradoListComponent]
})
export class SelectDoradoListModule {}
