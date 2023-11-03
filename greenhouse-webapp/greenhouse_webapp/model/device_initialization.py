import socket
import struct
import asyncio
import json
import time
import os
import netifaces

from model.udp_schemas import RegistrationSchema


script_dir = os.path.dirname(os.path.abspath(__file__))
manager_conf = os.path.join(script_dir, "conf/managerconf.json")


class ReceivedPing:
    def __init__(self, ip, name):
        self.time_received = self.update_time()
        self.ip = ip
        self.name = name
        
    def update_time(self):
        self.time_received = time.time()
        
    def check_time(self):
        if time.time() - self.time_received > 30:
            raise TimeoutError("The connection receive no pings for 30 seconds")
        

class DeviceManager:
    multicast_address = '224.0.2.4'
    port = 1337
    def __init__(self, loop: asyncio.BaseEventLoop):
        with open(manager_conf, 'r') as f:
            self.ip_address = json.loads(f.read())['host']
        self.loop = loop
        
        self.scans: dict[str,asyncio.Event] = dict()
        self.registration_queue = dict()
        
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('', self.port))
        
        receiving_group = socket.inet_aton(self.multicast_address)
        request = struct.pack('4sL', receiving_group, socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, request)
        
        self.sock.setblocking(False)
        
        self.active = True
        
    async def check_for_dead(self):
        while self.active:
            print("checking for dead")
            await asyncio.sleep(30)
            for mac, scan_object in self.scans.items():
                try: scan_object.check_time()
                except: self.scans.pop(mac)
        
    async def serve_management(self):
        while self.active:
            print("waiting for connection")
            data = await self.loop.sock_recv(self.sock, 1024)
            data = json.loads(data.decode())
            
            try:
                self.scans[data['ip']].update_time()
            except:
                scan_result = ReceivedPing(data['mac'], data['name'])
                self.scans[data['ip']] = scan_result
                
    async def send_registration(self, device_ip):
        if device_ip not in self.scans:
            raise Exception("The IP has not been detected by any scan!")
        message = RegistrationSchema(server_ip=self.ip_address).model_dump_json().encode()
        await self.loop.sock_sendall(self.sock, message)
        self.registration_queue[device_ip] = asyncio.Condition()
                
    def get_scans(self):
        return self.scans
    
    async def __call__(self):
        print("Now starting")
        await asyncio.gather(
            self.serve_management(),
            self.check_for_dead()
        )
    
    async def shutdown(self):
        self.active = False
        
                
            
        
    