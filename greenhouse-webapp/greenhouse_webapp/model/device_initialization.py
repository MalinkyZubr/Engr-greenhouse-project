import socket
import struct
import asyncio
import json
import time
import os
import netifaces

from typing import Union, Optional

from model.udp_schemas import RegistrationSchema
from model.data_interface import DatabaseInterface


script_dir = os.path.dirname(os.path.abspath(__file__))
manager_conf = os.path.join(script_dir, "conf/managerconf.json")


class Device:
    def __init__(self, ip: str, name: Optional[str]=None, id: Optional[int]=None, expidited: bool=False):
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
        self.time_received = time.time()
        
    def check_time(self) -> None:
        if time.time() - self.time_received > 30:
            raise TimeoutError("The connection receive no pings for 30 seconds")
        
    def json(self) -> dict[str, Union[str, int, bool]]:
        return {"ip":self.ip, "name":self.name, "id":self.id, "expidited":self.expidited}
        

class DeviceManager:
    multicast_address: str = '224.0.2.4'
    port = 1337
    def __init__(self, loop: asyncio.BaseEventLoop, database_interface: DatabaseInterface):
        self.database_interface: DatabaseInterface = database_interface
        
        with open(manager_conf, 'r') as f:
            self.ip_address = json.loads(f.read())['host']
        self.loop: asyncio.BaseEventLoop = loop
        
        self.scans: dict[str, Device] = dict() # devices who's UDP pings were detected
        
        self.registration_queue: dict[str, asyncio.Condition] = dict() # devices who are in the process of associating with the server
        self.active_device_list: dict[str, Device] = dict() # devices that have associated with the server and are active
        self.sleeping_device_list: dict[str, Device] = dict() # devices that have associated with the server and are inactive
        
        self.sock: socket.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('', self.port))
        
        receiving_group = socket.inet_aton(self.multicast_address)
        request = struct.pack('4sL', receiving_group, socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, request)
        
        self.sock.setblocking(False)
        
        self.active: bool = True
        
    async def check_for_dead(self) -> None:
        while self.active:
            print("checking for dead")
            await asyncio.sleep(30)
            for ip, scan_object in self.scans.items():
                try: scan_object.check_time()
                except: self.scans.pop(ip)
            for ip, scan_object in self.active_device_list.items():
                try: scan_object.check_time()
                except: 
                    self.active_device_list.pop(ip)
                    self.database_interface.execute("configureDevice", scan_object.id, status=False)
        
    async def serve_management(self) -> None:
        while self.active:
            print("waiting for connection")
            data: bytes = await self.loop.sock_recv(self.sock, 1024)
            data: dict[str, Union[str, int]] = json.loads(data.decode())
            
            try:
                self.scans[data['ip']].update_time()
            except:
                scan_result = Device(data['ip'], name=data['name'], id=data["id"], expidited=data["expidited"])
                self.scans[data['ip']] = scan_result
                
    async def send_registration(self, device_ip) -> None:
        if device_ip not in self.scans:
            raise Exception("The IP has not been detected by any scan!")
        message: RegistrationSchema = RegistrationSchema(server_ip=self.ip_address).model_dump_json().encode()
        await self.loop.sock_sendall(self.sock, message)
        self.registration_queue[device_ip] = asyncio.Condition()
        self.scans.pop(device_ip)
                
    def get_scans(self) -> dict[str, Device]:
        return self.scans

    async def shutdown(self) -> None:
        self.active: bool = False
        
                
            
        
    