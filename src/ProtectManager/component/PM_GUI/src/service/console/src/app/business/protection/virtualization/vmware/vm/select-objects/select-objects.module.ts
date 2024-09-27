import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SelectObjectsComponent } from './select-objects.component';
import { SelectedDisksModule } from './selected-disks/selected-disks.module';

@NgModule({
  declarations: [SelectObjectsComponent],
  imports: [BaseModule, SelectedDisksModule]
})
export class SelectObjectsModule {}
