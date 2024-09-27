import {
  Component,
  ComponentFactoryResolver,
  EventEmitter,
  Input,
  OnInit,
  Output,
  ViewChild,
  ViewContainerRef,
  OnDestroy
} from '@angular/core';
import { isFunction } from 'lodash';

@Component({
  selector: 'aui-create-container-modal',
  template: `
    <ng-container #dmroom></ng-container>
  `
})
export class CreateContainerModalComponent implements OnInit, OnDestroy {
  @ViewChild('dmroom', { read: ViewContainerRef, static: false })
  dmRoom: ViewContainerRef;
  @Input() config;
  @Input() option;
  @Input() resourceType;
  @Input() action;
  @Output() onConfigChange = new EventEmitter<any>();

  constructor(private cfr: ComponentFactoryResolver) {}

  ngOnDestroy() {
    if (this.dmRoom) {
      this.dmRoom.clear();
    }
  }

  ngOnInit() {
    const compFacotry = this.cfr.resolveComponentFactory(this.config.component);
    setTimeout(() => {
      const component: any = this.dmRoom.createComponent(compFacotry).instance;
      if (isFunction(component.initData)) {
        component.initData(this.option.data, this.resourceType, this.action);
      }
      this.onConfigChange.emit(component);
    });
  }
}

@Component({
  selector: 'aui-create-protect-modal',
  templateUrl: './create-protect-modal.component.html',
  styleUrls: ['./create-protect-modal.component.less']
})
export class CreateProtectModalComponent implements OnInit {
  type;
  option;
  protectionConfig;
  action;
  componentArr = [];
  constructor() {}

  onConfigChange(component) {
    this.componentArr.push(component);
  }

  ngOnInit() {}
}
