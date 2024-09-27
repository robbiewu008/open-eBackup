import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AddStorageComponent } from './add-storage.component';

@NgModule({
  declarations: [AddStorageComponent],
  imports: [CommonModule, BaseModule],
  exports: [AddStorageComponent]
})
export class AddStorageModule {}
