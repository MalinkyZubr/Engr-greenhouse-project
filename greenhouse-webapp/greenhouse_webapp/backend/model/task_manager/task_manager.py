import abc
import asyncio
import json
import logging

from typing import Awaitable, Union, Callable, Concatenate

import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")


class Task(abc.ABC):
    pass


class TaskManager[TaskSubjectGeneric]:
    def __init__(self, subject: TaskSubjectGeneric, identifier: str, run_interval: int = 10, logger: logging.Logger | None = None):
        self.identifier: str = identifier
        self.subject: TaskSubjectGeneric = subject
        self.logger: logging.Logger = logger
        self.run_interval: int = run_interval
        self.task_list: dict[str, Task] = list()
    
    def attach_task(self, task_id: str, task: Task):
        self.task_list.set(task_id ,task)
    
    async def run(self) -> Awaitable: # logging should exist on the loop somehow, integrate later. Each task should be its own new object maybe :(
        while True:
            for task in self.task_list:
                asyncio.ensure_future(task.run())
            await asyncio.sleep(self.run_interval)
    
    def start(self) -> None:
        self.run_task = asyncio.create_task(self.run())
    
    async def kill(self) -> None:
        self.run_task.cancel()
        
        try: await self.run_task
        except asyncio.CancelledError:
            if self.logger:
                self.logger.info(f"Cancelled all tasks for task manager {self.identifier}")
        return
            
    def get_logger(self) -> logging.Logger | None:
        return self.logger
    
    def get_subject(self) -> TaskSubjectGeneric:
        return self.subject
    
    
class Task[TaskSubjectGeneric](abc.ABC):
    def __init__(self, exception_handler: Callable[[Exception, Task], None] | None = None):
        self.logger: logging.Logger | None = None
        self.subject: TaskSubjectGeneric | None = None
        self.exception_handler = exception_handler

    @abc.abstractmethod
    async def task(self) -> Awaitable:
        pass
    
    async def run(self) -> Awaitable:
        if not self.subject:
            raise NotImplementedError("Subject not attached to task")
        try:
            await self.task()
        except Exception as e:
            if self.logger:
                self.logger.exception(str(e))
            if self.exception_handler:
                self.exception_handler(e, self)
            else:
                raise e
    
    def attach_task_manager(self, manager: TaskManager[TaskSubjectGeneric]) -> None:
        self.logger = manager.get_logger()
        self.subject = manager.get_subject()
        