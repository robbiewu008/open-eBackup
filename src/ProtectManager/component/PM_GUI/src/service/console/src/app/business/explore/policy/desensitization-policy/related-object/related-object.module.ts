import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { RelatedObjectComponent } from './related-object.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [RelatedObjectComponent],
  imports: [CommonModule, BaseModule],
  exports: [RelatedObjectComponent]
})
export class RelatedObjectModule {}
