import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared/base.module';
import { CopyDataScnSelectModule } from '../copy-data-scn-select/copy-data-scn-select.module';
import { CopyDataScnComponent } from './copy-data-scn.component';

@NgModule({
  declarations: [CopyDataScnComponent],
  imports: [BaseModule, CopyDataScnSelectModule],

  exports: [CopyDataScnComponent]
})
export class CopyDataScnModule {}
