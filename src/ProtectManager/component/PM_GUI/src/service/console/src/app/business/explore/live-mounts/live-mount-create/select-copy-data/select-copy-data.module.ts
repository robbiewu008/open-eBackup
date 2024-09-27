import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { SelectCopyDataComponent } from './select-copy-data.component';
import { CopyDataDetailModule } from 'app/shared/components/copy-data-detail/copy-data-detail.module';

@NgModule({
  declarations: [SelectCopyDataComponent],
  imports: [BaseModule, CopyDataDetailModule],
  exports: [SelectCopyDataComponent]
})
export class SelectCopyDataModule {}
