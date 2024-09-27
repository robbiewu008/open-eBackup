import { Component, EventEmitter, Input, OnInit, Output } from '@angular/core';
import { DataMap } from 'app/shared';
import { ProButton } from 'app/shared/components/pro-button/interface';

@Component({
  selector: 'aui-virtualization-group',
  templateUrl: './virtualization-group.component.html',
  styleUrls: ['./virtualization-group.component.less']
})
export class VirtualizationGroupComponent implements OnInit {
  @Input() treeSelection;
  @Input() subUnitType;
  @Output() updateTable = new EventEmitter();
  subType = DataMap.Resource_Type.vmGroup.value;

  header = '';
  activeIndex = '';
  treeData = [];
  expandedNodeList = [];

  optsConfig: ProButton[] = [];

  extParams = {};
  constructor() {}
  ngOnInit() {}
  updateTable1(e) {
    this.updateTable.emit({ total: e.total });
  }
}
