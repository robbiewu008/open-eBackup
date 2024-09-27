import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { GetTacticsComponent } from './get-tactics.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [GetTacticsComponent],
  imports: [CommonModule, BaseModule],
  exports: [GetTacticsComponent]
})
export class GetTacticsModule {}
