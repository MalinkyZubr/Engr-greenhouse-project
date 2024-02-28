import abc
import asyncio
import json
import logging

from typing import Awaitable, Union, Callable, Concatenate
from pydantic import ValidationError

import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from model.task_manager.task_subject import TaskSubjectGeneric


class Task(abc.ABC):
    pass


class TaskManager[TaskSubjectGeneric]:
    def __init__(self, subject: TaskSubject, run_interval: int = 10, logger: logging.Logger | None = None):
        self.subject = subject
        self.logger = logger
        self.run_interval = run_interval
        self.task_list: dict[str, Task] = list()
    
    def attach_task(self, task_id: str, task: Task):
        self.task_list.set(task_id ,task)
    
    async def run(self) -> Awaitable: # logging should exist on the loop somehow, integrate later. Each task should be its own new object maybe :(
        while True:
            for task in self.task_list:
                asyncio.ensure_future(task)
            await asyncio.sleep(self.run_interval)
    
    def start(self) -> None:
        self.run_task = asyncio.create_task(self.run())
    
    async def kill(self) -> None:
        self.run_task.cancel()
        
        try: await self.run_task
        except asyncio.CancelledError:
            return
            
    def get_logger(self) -> logging.Logger | None:
        return self.logger
    
    def get_subject(self) -> TaskSubject:
        return self.subject
    
    
class Task(abc.ABC):
    def __init__(self, exception_handler: Callable[[Exception, Task], None] | None = None):
        self.logger: logging.Logger | None = None
        self.subject: TaskSubject | None = None
        self.exception_handler = exception_handler

    @abc.abstractmethod
    async def task(self) -> Awaitable:
        pass
    
    async def run(self) -> Awaitable:
        try:
            await self.task()
        except Exception as e:
            if self.logger:
                self.logger.exception(e.args)
            if self.exception_handler:
                self.exception_handler(e, self)
            else:
                raise e
    
    def attach_task_manager(self, manager: TaskManager) -> None:
        self.logger = manager.get_logger()
        self.subject = manager.get_subject()
        