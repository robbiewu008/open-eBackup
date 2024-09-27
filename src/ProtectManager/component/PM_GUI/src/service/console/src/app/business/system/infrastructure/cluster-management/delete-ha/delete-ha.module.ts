import { CommonModule } from '@angular/common';
import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { DeleteHaComponent } from './delete-ha.component';

@NgModule({
  declarations: [DeleteHaComponent],
  imports: [CommonModule, BaseModule],
  exports: [DeleteHaComponent]
})
export class DeleteHaModule {}
