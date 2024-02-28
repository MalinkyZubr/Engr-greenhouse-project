import abc
import asyncio
import json

from typing import Awaitable, Union
from pydantic import ValidationError

import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from model.task_manager.task_manager import Task, TaskManager
from model.device_manager.scanner.udp_schemas import ConnectRequestSchema
from model.device_manager.device import Device, DeviceContainer
            
            
class ScanListener(Task[DeviceContainer]):
    async def receive_request(self) -> Awaitable:
        data: bytes = await self.loop.sock_recv(self.sock, 1024)
        data: dict[str, Union[str, int]] = json.loads(data.decode())
        
        data_validated = ConnectRequestSchema(**data)
        
        requesting_ip: str = data["ip"]
        
        return requesting_ip, data_validated
    
    async def task(self) -> Awaitable:
        requesting_ip, data: [str, ConnectRequestSchema] = await self.receive_request()
            
        if(requesting_ip in self.scans):
            self.subject.get(requesting_ip).update_time()
        else:
            scan_result = Device(**data)
            self.subject.set(requesting_ip, scan_result)
    
    
class CheckDeadScans(Task[DeviceContainer]):
    async def task(self) -> Awaitable:
        """check for dead connections, so they can be removed. Dead connections are those that fail the "is_dead" function of device class
        """
        for ip, scan_object in self.subject.items():
            if scan_object.is_dead():
                self.subject.pop(ip)
                

class NonAssociatedDeviceManager(DeviceManager):
    async def send_registration(self, device_ip: str) -> None:
        """when the client requests to add a device to the server, this will send the registration request to the device so it can proceed with authentication and setup

        Args:
            device_ip (str): ip of desired device

        Raises:
            IndexError: if the IP is not in the registration queue, this error is raised
        """
        if device_ip not in self.scans:
            raise IndexError("The IP has not been detected by any scan!")
        message: RegistrationSchema = RegistrationSchema(server_ip=self.ip_address, server_port=self.port).model_dump_json().encode()
        await self.loop.sock_sendall(self.sock, message)
        self.registration_queue[device_ip] = asyncio.Condition() # this should be a trait of the device
        self.scans.pop(device_ip)


    