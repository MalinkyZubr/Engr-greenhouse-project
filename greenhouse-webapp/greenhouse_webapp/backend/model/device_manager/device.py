import abc
from typing import Union, Optional, Awaitable, Literal
from pydantic_core import ValidationError
from socket import socket
import asyncio
import time
import sys

from model.device_manager.device_manager.udp_schemas import ConnectRequestSchema, RegistrationSchema
from model.database.database_manager import get_device, QueryByID, get_preset, METADATA_OBJECT


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
        self.state: DeviceState
        
        self.selected_state: DeviceState = None
        
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
    
    async def execute_state(self) -> Awaitable:
        self.selected_state.state_action()
    
    def state_set_scan(self, udp_socket: socket):
        self.selected_state = ScannedState(self, udp_socket)
        
    
# might implement if the device class needs refactoring
class DeviceState:
    dead_time_interval: int | None = None
    
    def __init__(self, context: Device):
        self.context: Device = context
    
    @abc.abstractmethod
    async def state_action(self) -> Awaitable:
        pass
    
    @abc.abstractmethod
    def state_failure_action(self) -> Awaitable:
        pass
    
    @abc.abstractmethod
    def dead_action(self) -> None:
        pass
    

class ScannedState(DeviceState):
    def __init__(self, context: Device, sock: socket):
        super(context)
        self.sock: socket = sock
        
    async def state_action(self) -> Awaitable:
        message: RegistrationSchema = RegistrationSchema(server_ip=self.ip_address, server_port=self.port).model_dump_json().encode()
        await asyncio.get_event_loop().sock_sendall(self.sock, message)
        

class RegistrationRequested(DeviceState):
    def __init__(self, context: Device, await_condition: asyncio.Condition):
        super(context)
        self.await_contiditon: asyncio.Condition = await_condition
        
    async def state_action(self) -> Awaitable:
        reported_device_id: str = self.context.id
        
        database_device_data: tuple = get_device.execute(QueryByID(id=self.context.id))
        
        # check to see if the device knows its device id and it doesnt have data in the database. This means the device was unregistered while the device was disconnected
        if reported_device_id and not database_device_data: # this is an edge case. If it becomes necessary ill implement it later
            true_device_id: str = reported_device_id
            
            await device_unregister.call(source_ip)
            # do something with reregistration here, I dont frickin know
            return
        
        # check to see if the device knows its device id, and it does have an entry in the database
        elif reported_device_id and database_device_data:
            true_device_id: str = reported_device_id
            
            server_preset_response: Optional[DeviceRegistrationPreset] = None
            
            database_preset_id: int = database_device_data["PresetID"]
            database_preset_information: METADATA_OBJECT = get_preset.execute(QueryByID(id=database_preset_id))

            if database_preset_information:
                server_preset_response = DeviceRegistrationPreset(preset_id=database_preset_id,
                    temperature=database_preset_information[2],
                    humidity=database_preset_information[3],
                    moisture=database_preset_information[4],
                    hours_daylight=database_preset_information[5])
                
            database_project_id: int = database_device_data[5]
            
            response: DeviceRegistrationResponse = DeviceRegistrationResponse(device_id = true_device_id,
                device_name = database_device_data[1],
                project_id = database_project_id,
                preset = server_preset_response,
                reference_time = time.time())
        
        # if the device has never been configured before (if no reported ID)
        else:
            router.database_connector.execute("registerDevice", source_ip, None, None, "IDLE") # device name initially set to the ip
            true_device_id: int = router.database_connector.execute("getDeviceID", source_ip)
            
            response = DeviceRegistrationResponse(device_id = true_device_id,
                reference_time=time.time())
        
        # check for expidited flag here
        device_info: Device = router.device_manager.registration_queue.pop(source_ip)
        
        if not device_info.expidited: # if the process is expidited as specified by the device, that means the user never made an explicit request. Thus this step can be skipped
            router.device_manager.registration_queue[source_ip].notify_all()
            
        router.device_manager.active_device_list[true_device_id] = device_info
        self.await_contiditon.notify_all() # only at the end
        
    
    
    
class DeviceActive(DeviceState):
    pass

    
class DeviceIdle(DeviceState):
    pass

    
class DeviceDisconnect(DeviceState):
    pass


class DeviceError(DeviceState):
    pass
    
    
# class DeviceContainer(TaskSubject):
#     def __init__(self):
#         self.devices: dict[Device] = dict()
        
#     def add_device(self, device_id: str, device: Device) -> None:
#         self.devices.set(device_id, device)
        
#     def remove_device(self, device_id: str) -> None:
        
        
DeviceContainer = dict[str, Device]


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