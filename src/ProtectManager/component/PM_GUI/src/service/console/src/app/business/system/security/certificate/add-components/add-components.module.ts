import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { UploadModule } from '@iux/live';
import { BaseModule } from 'app/shared';
import { AddComponentsComponent } from './add-components.component';

@NgModule({
  declarations: [AddComponentsComponent],
  imports: [CommonModule, BaseModule, UploadModule],

  exports: [AddComponentsComponent]
})
export class AddComponentsModule {}
