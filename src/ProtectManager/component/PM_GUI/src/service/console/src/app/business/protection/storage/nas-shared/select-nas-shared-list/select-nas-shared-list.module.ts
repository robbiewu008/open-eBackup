import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { SelectNasSharedListComponent } from './select-nas-shared-list.component';
import { BaseModule } from 'app/shared';
import { SelectProtectObjectsModule } from 'app/shared/components/select-protect-objects/select-protect-objects.module';

@NgModule({
  declarations: [SelectNasSharedListComponent],
  imports: [CommonModule, BaseModule, SelectProtectObjectsModule],
  exports: [SelectNasSharedListComponent]
})
export class SelectNasSharedListModule {}
