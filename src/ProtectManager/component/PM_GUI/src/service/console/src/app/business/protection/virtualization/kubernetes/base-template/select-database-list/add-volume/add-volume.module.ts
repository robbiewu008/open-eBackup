import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AddVolumeComponent } from './add-volume.component';

@NgModule({
  declarations: [AddVolumeComponent],
  imports: [CommonModule, BaseModule]
})
export class AddVolumeModule {}
