import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';
import { SelectGaussdbTListComponent } from './select-guassdb-t-list.component';

@NgModule({
  declarations: [SelectGaussdbTListComponent],
  imports: [CommonModule, BaseModule, SelectProtectObjectsModule],
  exports: [SelectGaussdbTListComponent]
})
export class SelectGaussdbTListModule {}
