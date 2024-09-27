import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { AdvancedComponent } from './advanced.component';

@NgModule({
  declarations: [AdvancedComponent],
  imports: [BaseModule]
})
export class AdvancedModule {}
