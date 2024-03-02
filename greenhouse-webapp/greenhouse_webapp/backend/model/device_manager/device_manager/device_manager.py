import abc
import asyncio
import json
import socket
import struct

from typing import Awaitable, Union
from pydantic import ValidationError

import sys
sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from model.task_manager.task_manager import Task, TaskManager
from model.device_manager.device_manager.udp_schemas import ConnectRequestSchema
from model.device_manager.device import Device, DeviceContainer
from utilities.config_reader import get_config


class ScanUDPSocket:
    buffer_size: int = 1024
    def __init__(self):
        config = get_config("device_scanner") # standardize the config getting, return some sort of class to make things more solid and stable
        port = config["listening_port"]
        multicast_address = config["multicastaddress"]
        
        self.sock: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('', port))
        
        receiving_group: bytes = socket.inet_aton(multicast_address)
        request = struct.pack('4sL', receiving_group, socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, request)
        
        self.sock.setblocking(False)
        
    async def receive(self) -> dict[str, Union[str, int]]:
        data: bytes = await asyncio.get_event_loop().sock_recv(self.sock, self.buffer_size)
        data: dict[str, Union[str, int]] = json.loads(data.decode())
        
        return data
            
            
class ScanListener(Task[DeviceContainer]):
    def __init__(self):
        self.udp_socket: ScanUDPSocket = ScanUDPSocket()
        
    async def receive_request(self) -> Awaitable:
        data = await self.udp_socket.receive()
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


    