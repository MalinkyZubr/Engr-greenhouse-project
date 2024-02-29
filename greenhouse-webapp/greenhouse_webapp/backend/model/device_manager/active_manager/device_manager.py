import abc
import asyncio
import json

from typing import Awaitable, Union
from pydantic import ValidationError

import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from model.task_manager.task_manager import Task, TaskManager
from model.device_manager.device import Device
  
        
    
class AssociatedDeviceContainer(dict[str, Device]):
    def __init__(self, database_interface): # create a type for this
        self.database_interface = database_interface
        
    def pop(self, key: str):
        super().pop(key)
        self.database_interface.do_thing()




    # async def send_registration(self, device_ip: str) -> None:
    #     """when the client requests to add a device to the server, this will send the registration request to the device so it can proceed with authentication and setup

    #     Args:
    #         device_ip (str): ip of desired device

    #     Raises:
    #         IndexError: if the IP is not in the registration queue, this error is raised
    #     """
    #     if device_ip not in self.scans:
    #         raise IndexError("The IP has not been detected by any scan!")
    #     message: RegistrationSchema = RegistrationSchema(server_ip=self.ip_address, server_port=self.port).model_dump_json().encode()
    #     await self.loop.sock_sendall(self.sock, message)
    #     self.registration_queue[device_ip] = asyncio.Condition() # this should be a trait of the device
    #     self.scans.pop(device_ip)


    