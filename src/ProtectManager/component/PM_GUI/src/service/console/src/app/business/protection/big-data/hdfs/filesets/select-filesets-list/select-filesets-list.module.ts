import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SelectFilesetsListComponent } from './select-filesets-list.component';
import { BaseModule } from 'app/shared';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';

@NgModule({
  declarations: [SelectFilesetsListComponent],
  imports: [CommonModule, BaseModule, SelectProtectObjectsModule],
  exports: [SelectFilesetsListComponent]
})
export class SelectFilesetsListModule {}
