import { Component, OnInit } from '@angular/core';

@Component({
  selector: 'aui-oracle-summary',
  templateUrl: './summary.component.html',
  styleUrls: ['./summary.component.less']
})
export class SummaryComponent implements OnInit {
  targetName = 'Oracle01';
  mountTo = 'OrinalLoacation';
  location = 'Host001\\Oracle01';
  copyData = '2019/10/31 08:30AM';
  maskEngine = 'Mask in Local Cluster';
  scriptTool = 'Desensitizing Tool of Finacial Department.py (168k)';
  constructor() {}

  ngOnInit() {}
}
