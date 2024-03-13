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
from backend.model.devices.udp_schemas import ConnectRequestSchema
from model.devices.device import Device
from utilities.config_reader import get_config


DeviceContainer = dict[str, Device]


class ScanUDPSocket:
    buffer_size: int = 1024
    def __init__(self, listening_port, multicast_address):
        self.sock: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('', listening_port))
        
        receiving_group: bytes = socket.inet_aton(multicast_address)
        request = struct.pack('4sL', receiving_group, socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, request)
        
        self.sock.setblocking(False)
        self.socket_lock: asyncio.Lock = asyncio.Lock()
        
    async def receive(self) -> dict[str, Union[str, int]]:
        with self.socket_lock:
            data: bytes = await asyncio.get_event_loop().sock_recv(self.sock, self.buffer_size)
            data: dict[str, Union[str, int]] = json.loads(data.decode())
        
        return data
    
    @staticmethod
    async def send(message: str, destination_ip: str) -> None:
        send_sock: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        await asyncio.get_event_loop().sock_sendto(bytes(message), destination_ip)
        
            
class ScanListener(Task[DeviceContainer]):
    def __init__(self):
        self.udp_socket: ScanUDPSocket = ScanUDPSocket()
        
    async def receive_request(self) -> Awaitable:
        data = await self.udp_socket.receive()
        data_validated = ConnectRequestSchema(**data)
        
        requesting_ip: str = data["ip"]
        
        return requesting_ip, data_validated
    
    async def task(self) -> Awaitable:
        requesting_ip: str
        data: ConnectRequestSchema
        
        requesting_ip, data = await self.receive_request()
            
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
    


    