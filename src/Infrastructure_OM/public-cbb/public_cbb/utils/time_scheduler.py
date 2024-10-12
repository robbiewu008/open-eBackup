#!/usr/bin/env python
# _*_ coding:utf-8 _*_

import logging
from pytz import utc
from apscheduler.schedulers.background import BackgroundScheduler

from public_cbb.config.global_config import get_settings

settings = get_settings()

_executors = {
    'default': {'type': 'threadpool', 'max_workers': settings.TIME_SCHEDULER_WORKS},
}

_job_defaults = {
    'coalesce': True,
    'max_instances': 1
}

time_scheduler = BackgroundScheduler()
time_scheduler.configure(executors=_executors, job_defaults=_job_defaults, timezone=utc)
logging.getLogger('apscheduler.executors.default').setLevel(logging.WARNING)  # 避免打印无效的日志
