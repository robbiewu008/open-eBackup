import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { CreateTagComponent } from './create-tag.component';
import { BaseModule } from 'app/shared/base.module';

@NgModule({
  declarations: [CreateTagComponent],
  imports: [CommonModule, BaseModule]
})
export class CreateTagModule {}
