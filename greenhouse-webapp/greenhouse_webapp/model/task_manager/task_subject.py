import abc
from typing import TypeVar


class TaskSubject(abc.ABC):
    pass


TaskSubjectGeneric = TypeVar("TaskSubjectGeneric", TaskSubject)