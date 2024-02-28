import abc
from typing import Union, Optional
from pydantic_core import ValidationError
import time
import sys

sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from model.task_manager.task_subject import TaskSubject


class DeviceState:
    pass


class Device:
    def __init__(self, ip: str, name: Optional[str]=None, id: Optional[int]=None, expidited: bool=False):
        """Low level class for storing information about active connections to the server. Used to provide registration with server as well

        Args:
            ip (str): ip address of the device
            name (Optional[str], optional): name for the device to display on frontend. Only set after registration. Defaults to None.
            id (Optional[int], optional): id to use for data accessing and identification by other parts of program. Only set after registration. Defaults to None.
            expidited (bool, optional): The expidited flag identifies if the device making the conneciton request has already connected to the server, and has a saved ID on its flash memory.
            This will allow it to skip manual registration and immediately associate to the server. Defaults to False.
        """
        self.time_received: int = self.update_time()
        self.ip: str = ip
        self.name: str = name
        self.id: int = id
        self.is_associated: bool = False
        self.expidited: bool = expidited
        
    def associate(self) -> None:
        self.associated = True
        
    def dissociate(self) -> None:
        self.associated = False
        
    def set_id(self, id: int) -> None:
        self.id = id
        
    def update_time(self) -> None:
        """reset the timer for disconnecting the device
        """
        self.time_received = time.time()
        
    def is_dead(self) -> bool:
        """check if the timeout threshold was exceeded, if no messages have been received in a while

        Raises:
            TimeoutError: raised if timeout threshold exceeded
        """
        return time.time() - self.time_received > 30
        
    def json(self) -> dict[str, Union[str, int, bool]]:
        return {"ip":self.ip, "name":self.name, "id":self.id, "expidited":self.expidited}
    
    
# might implement if the device class needs refactoring
class DeviceState:
    pass
    
    
# class DeviceContainer(TaskSubject):
#     def __init__(self):
#         self.devices: dict[Device] = dict()
        
#     def add_device(self, device_id: str, device: Device) -> None:
#         self.devices.set(device_id, device)
        
#     def remove_device(self, device_id: str) -> None:
        
        
DeviceContainer = dict[str, Device]