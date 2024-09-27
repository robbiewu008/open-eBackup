import { NgModule } from '@angular/core';
import { SelectResourceComponent } from './select-resource.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [SelectResourceComponent],
  imports: [BaseModule],
  exports: [SelectResourceComponent]
})
export class SelectResourceModule {}
