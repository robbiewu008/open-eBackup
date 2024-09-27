import { NgModule } from '@angular/core';
import { BaseModule } from 'app/shared';
import { CopyDataModule as CommonCopyData } from 'app/shared/components/copy-data/copy-data.module';
import { CopyDataComponent } from './copy-data.component';

@NgModule({
  declarations: [CopyDataComponent],
  imports: [BaseModule, CommonCopyData]
})
export class CopyDataModule {}
