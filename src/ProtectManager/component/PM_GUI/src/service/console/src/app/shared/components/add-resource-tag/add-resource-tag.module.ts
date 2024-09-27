import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { AddResourceTagComponent } from './add-resource-tag.component';
import { TransferModule } from '@iux/live';

@NgModule({
  declarations: [AddResourceTagComponent],
  imports: [CommonModule, BaseModule, TransferModule],
  exports: [AddResourceTagComponent]
})
export class AddResourceTagModule {}
