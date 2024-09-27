import logging
import time

logging_logger = logging.getLogger(__name__)


def retry_func(func, retry_times=5, delay=1, logger=logging_logger):
    def decorate():
        _times = retry_times
        while _times:
            try:
                return func()
            except Exception as e:
                _times -= 1
                if not _times:
                    raise Exception('Retry failed.')
                if logger is not None:
                    logger.exception(f'{e}, retrying in {delay} seconds...')
                time.sleep(delay)
    return decorate
