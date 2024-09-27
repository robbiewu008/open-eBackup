import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { AssociatedFilesetComponent } from './associated-fileset.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [AssociatedFilesetComponent],
  imports: [CommonModule, BaseModule],
  exports: [AssociatedFilesetComponent]
})
export class AssociatedFilesetModule {}
