import { NgModule } from '@angular/core';
import { CommonModule } from '@angular/common';
import { UserGuideComponent } from './user-guide.component';
import { BaseModule } from 'app/shared';

@NgModule({
  declarations: [UserGuideComponent],
  imports: [CommonModule, BaseModule],
  exports: [UserGuideComponent]
})
export class UserGuideModule {}
