import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { BaseModule } from 'app/shared';
import { CreateFilesetComponent } from './create-fileset.component';
import { ResourceFilterModule } from 'app/shared/components/resource-filter/resource-filter.module';

@NgModule({
  declarations: [CreateFilesetComponent],
  imports: [CommonModule, BaseModule, ResourceFilterModule],
  exports: [CreateFilesetComponent]
})
export class CreateFilesetModule {}
