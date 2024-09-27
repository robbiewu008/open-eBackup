import {
  Component,
  Input,
  OnInit,
  TemplateRef,
  ViewChild
} from '@angular/core';
import { ModalRef } from '@iux/live';

@Component({
  selector: 'aui-modify-template-info',
  templateUrl: './modify-template-info.component.html',
  styleUrls: ['./modify-template-info.component.less']
})
export class ModifyTemplateInfoComponent implements OnInit {
  @Input() rowItem;
  @ViewChild('modifyHeaderTpl', { static: true })
  modifyHeaderTpl: TemplateRef<any>;

  constructor(private modal: ModalRef) {}

  ngOnInit() {
    this.initHeader();
  }

  initHeader() {
    this.modal.setProperty({ lvHeader: this.modifyHeaderTpl });
  }
}
