import socket
import struct
import asyncio
import json
import time


class ReceivedPing:
    def __init__(self, ip, name):
        self.time_received = self.update_time()
        self.ip = ip
        self.name = name
        
    def update_time():
        self.time_received = time.time()
        
    def check_time():
        if time.time() - time.received > 30:
            raise TimeoutError("The connection receive no pings for 30 seconds")
        

class DeviceManager:
    mutlicast_address = '224.0.2.4'
    port = 1337
    def __init__(self, loop):
        self.loop = loop
        self.scans = dict()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('', self.port))
        
        receiving_group = socket.inet_aton(self.multicast_address)
        request = struct.pack('4sL', receiving_group, socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, request)
        
        self.sock.setblocking(False)
        
    async def check_for_dead(self):
        while True:
            await asyncio.sleep(30)
            for mac, scan_object in self.scans.items():
                try: scan_object.check_time()
                except: self.scans.pop(mac)
        
    async def serve_management(self):
        while True:
            data = await self.loop.sock_recv(self.sock, 1024)
            data = json.loads(data.decode())
            
            try:
                self.scans[data['mac']].update_time()
            except:
                scan_result = ReceivedPing(data['ip'], data['name'])
                self.scans[data['mac']] = scan_result
    
    async def __call__(self):
        await asyncio.gather(
            self.serve_management(),
            self.check_for_dead()
        )
            
        
                
            
        
    