# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
import asyncio
import contextvars
import inspect
from asyncio.events import AbstractEventLoop
from asyncio.futures import Future
from concurrent.futures.thread import ThreadPoolExecutor
from contextlib import asynccontextmanager, AsyncExitStack, contextmanager
from datetime import timedelta, datetime
from functools import wraps
from typing import Any, Optional, Callable, TypeVar

from fastapi import APIRouter, FastAPI, Depends, routing
from fastapi.dependencies.utils import is_gen_callable

from app.common import logger

log = logger.get_logger(__name__)

T = TypeVar('T')
MILLI_SECONDS = timedelta(milliseconds=1)
ONE_THOUSAND_MILLISECONDS = 1000

DEFAULT_ASYNC_POOL = ThreadPoolExecutor(128, thread_name_prefix="default_async_pool")
HTTP_ASYNC_POOL = ThreadPoolExecutor(64, thread_name_prefix="http_async_pool")


async def run_in_pool(method, args=None, kwargs=None, pool=None):
    start_time = datetime.now()
    loop = asyncio.get_running_loop()
    future = loop.create_future()
    args = args or ()
    kwargs = kwargs or {}
    pool = pool or DEFAULT_ASYNC_POOL

    def callback(task: Future):
        exception = task.exception()
        if exception is not None:
            log.exception(f"run task failed. method: {method}", exc_info=exception)

    try:
        pool.submit(execute, method, loop, future, start_time, *args, **kwargs).add_done_callback(callback)
    except Exception as ex:
        log.exception(f"submit task failed. method:{method}")
        future.set_exception(ex)
    return await future


def execute(method, loop: AbstractEventLoop, future: Future, start_time, *args, **kwargs):
    def call_soon(callback, argument):
        try:
            loop.call_soon_threadsafe(callback, argument)
        except Exception:
            log.exception("call soon failed.")
            raise

    def wrap(callback, action):
        def wrapper(*_, **__):
            try:
                return callback(*_, **__)
            except Exception:
                log.exception(f"{action} failed.")
                raise

        return wrapper

    try:
        running_time = datetime.now()
        waiting_time = (running_time - start_time) / MILLI_SECONDS
        if waiting_time > ONE_THOUSAND_MILLISECONDS:
            log.info(f"waiting time(method:{method}, cost:{waiting_time}ms) too long.")
        result = method(*args, **kwargs)
        call_soon(wrap(future.set_result, 'feedback result'), result)
    except Exception as ex:
        call_soon(wrap(future.set_exception, 'feedback error'), ex)


@asynccontextmanager
async def context_manager_in_pool(cm: Any, pool: ThreadPoolExecutor = None) -> Any:
    try:
        yield await run_in_pool(cm.__enter__, pool=pool)
    except Exception as e:
        ok = await run_in_pool(cm.__exit__, [type(e), e, None])
        if not ok:
            raise e
    else:
        await run_in_pool(cm.__exit__, [None] * 3, pool=pool)


COMMON_ASYNC_EXIT_STACK = contextvars.ContextVar('CommonAsyncExitStack')


class AsyncContextExitStack(AsyncExitStack):
    def __init__(self):
        super(AsyncContextExitStack, self).__init__()
        token = COMMON_ASYNC_EXIT_STACK.set(self)

        async def reset(*_):
            COMMON_ASYNC_EXIT_STACK.reset(token)

        self.push_async_exit(reset)


routing.AsyncExitStack = AsyncContextExitStack


def to_async_method(method: 'T', pool: ThreadPoolExecutor = None, stack: AsyncExitStack = None) -> 'T':
    if inspect.iscoroutinefunction(method):
        return method
    pool = pool if pool is not None else DEFAULT_ASYNC_POOL

    @wraps(method)
    async def wrapper(*args, **kwargs):
        if is_gen_callable(method):
            cm = context_manager_in_pool(contextmanager(method)(*args, **kwargs))
            async_stack = COMMON_ASYNC_EXIT_STACK.get(stack)
            if async_stack is None:
                raise Exception("not provide async exit stack")
            return await async_stack.enter_async_context(cm)
        return await run_in_pool(method, args, kwargs, pool=pool)

    return wrapper


def async_route(pool: ThreadPoolExecutor = None):
    pool = pool if pool is not None else HTTP_ASYNC_POOL
    return async_decorate(APIRouter(), 'get', 'put', 'post', 'delete', 'on_event', pool=pool)


def async_fast_api(pool: ThreadPoolExecutor = None):
    pool = pool if pool is not None else HTTP_ASYNC_POOL
    return async_decorate(FastAPI(), 'get', 'put', 'post', 'delete', 'on_event', pool=pool)


def async_decorate(obj: 'T', *item_list: str, pool: ThreadPoolExecutor = None) -> 'T':
    for item in item_list:
        value = getattr(obj, item, None)
        if callable(value):
            setattr(obj, item, to_async_decorator(value, pool=pool))
    return obj


def async_depend(dependency: Optional[Callable] = None, *, use_cache: bool = True):
    if dependency is None:
        return Depends(use_cache=use_cache)
    return Depends(to_async_method(dependency, pool=HTTP_ASYNC_POOL), use_cache=use_cache)


def to_async_decorator(method: Callable, pool: ThreadPoolExecutor = None):
    if inspect.iscoroutinefunction(method):
        return method

    def decorate(func):
        if inspect.iscoroutinefunction(func):
            return func

        async_method = to_async_method(func, pool=pool)

        @wraps(func)
        async def wrapper(*args, **kwargs):
            return await async_method(*args, **kwargs)

        return wrapper

    def decorator(*args, **kwargs):
        raw_decorator = method(*args, **kwargs)

        def async_decorator(func):
            async_method = decorate(func)
            return raw_decorator(async_method)

        return async_decorator

    decorator.__name__ = method.__name__
    return decorator
