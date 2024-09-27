import { Component, OnInit, ViewChild } from '@angular/core';
import { JobResourceComponent } from './job-resource/job-resource.component';
import { JobStatisticsComponent } from './job-statistics/job-statistics.component';

@Component({
  selector: 'aui-job',
  templateUrl: './job.component.html',
  styleUrls: ['./job.component.less']
})
export class JobComponent implements OnInit {
  @ViewChild(JobStatisticsComponent, { static: false })
  jobStatisticsComponent: JobStatisticsComponent;
  @ViewChild(JobResourceComponent, { static: false })
  jobResourceComponent: JobResourceComponent;

  constructor() {}

  ngOnInit() {}

  change() {
    this.jobStatisticsComponent.getData();
    this.jobResourceComponent.getData();
  }
}
