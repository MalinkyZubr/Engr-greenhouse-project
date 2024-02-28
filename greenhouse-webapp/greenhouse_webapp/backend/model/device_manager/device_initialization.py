import socket
import struct
import asyncio
import json
import time
import os
import sys
import netifaces
import abc

from typing import Union, Optional
from pydantic_core import ValidationError

from greenhouse_webapp.model.device_manager.scanner.udp_schemas import RegistrationSchema, ConnectRequestSchema
from greenhouse_webapp.model.database.data_interface import DatabaseInterface

sys.path.append("/home/malinkyzubr/Desktop/purdue-stuff/Fall-2023/ENGR-101/Engineering-Design-Project/greenhouse-webapp/greenhouse_webapp")
from model.task_manager.task_manager import Task



script_dir = os.path.dirname(os.path.abspath(__file__))
manager_conf = os.path.join(script_dir, "conf/managerconf.json")


class CheckForDeadTask()
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
        

class DeviceManager:
    multicast_address: str = '224.0.2.4'
    port = 1337
    def __init__(self, database_interface: DatabaseInterface):
        """class for managing low level socket operations, association, and connection statuses

        Args:
            loop (asyncio.BaseEventLoop): event loop for async operations
            database_interface (DatabaseInterface): interface to the database for managing data
        """
        self.database_interface: DatabaseInterface = database_interface
        
        with open(manager_conf, 'r') as f:
            self.ip_address = json.loads(f.read())['host']
        
        self.scans: dict[str, Device] = dict() # devices who's UDP pings were detected
        
        self.registration_queue: dict[str, asyncio.Condition] = dict() # devices who are in the process of associating with the server
        self.active_device_list: dict[str, Device] = dict() # devices that have associated with the server and are active
        
        self.sock: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('', self.port))
        
        receiving_group = socket.inet_aton(self.multicast_address)
        request = struct.pack('4sL', receiving_group, socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, request)
        
        self.sock.setblocking(False)
        
        self.active: bool = True
        
    async def check_for_dead(self) -> None:
        """check for dead connections, so they can be removed. Dead connections are those that fail the "is_dead" function of device class
        """
        while self.active:
            print("checking for dead")
            await asyncio.sleep(30)
            for ip, scan_object in self.scans.items():
                if scan_object.is_dead():
                    self.scans.pop(ip)
            for ip, scan_object in self.active_device_list.items():
                if scan_object.is_dead():
                    self.active_device_list.pop(ip)
                    self.database_interface.execute("configureDevice", scan_object.id, status="DISCONNECTED")
        
    async def serve_management(self) -> None:
        """listen for UDP requests to the mutlicast server address, places devices into the registration queue in waiting to be associated to the server by a client
        """
        while self.active:
            print("waiting for connection")
            data: bytes = await self.loop.sock_recv(self.sock, 1024)
            data: dict[str, Union[str, int]] = json.loads(data.decode())
            
            requesting_ip: str = data["ip"]
            
            if(requesting_ip in self.scans):
                self.scans[data['ip']].update_time()
            else:
                try:
                    scan_result_validated = ConnectRequestSchema(**data)
                    scan_result = Device(**data)
                    self.scans[data['ip']] = scan_result
                except ValidationError:
                    print("received registration schema failed validation")
                
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
        self.registration_queue[device_ip] = asyncio.Condition()
        self.scans.pop(device_ip)
                
    def get_scans(self) -> dict[str, Device]:
        """gets the scan list of the manager

        Returns:
            dict[str, Device]: list of scans
        """
        return self.scans

    async def shutdown(self) -> None:
        self.active: bool = False
        
                
            
        
    