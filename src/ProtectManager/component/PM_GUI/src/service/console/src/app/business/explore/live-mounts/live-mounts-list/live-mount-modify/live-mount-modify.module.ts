import { NgModule } from '@angular/core';
import { LiveMountModifyComponent } from './live-mount-modify.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [LiveMountModifyComponent],
  imports: [BaseModule],
  exports: [LiveMountModifyComponent]
})
export class LiveMountModifyModule {}
