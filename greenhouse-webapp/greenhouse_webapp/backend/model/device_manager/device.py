import abc
from typing import Union, Optional, Awaitable, Literal, override
from pydantic_core import ValidationError
from fastapi import Request, APIRouter, HTTPException
from socket import socket
import asyncio
import time
import sys

from model.device_manager.device_manager.udp_schemas import ConnectRequestSchema, RegistrationSchema
from model.database.database_manager import DatabaseInterface, QueryByID, METADATA_OBJECT, RegisterDeviceQuery, QueryByIP
from model.device_manager.device_manager.device_manager import ScanUDPSocket
from controller.device_management.schemas import DeviceRegistrationPreset, DeviceRegistrationResponse

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
        self.conditions: dict[str, asyncio.Condition] = dict()
        
        self.time_received: int = self.update_time()
        self.ip: str = ip
        self.name: str = name
        self.id: int = id
        self.expidited: bool = expidited
        
        self.selected_state: DeviceState | None = None
        
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
    
    async def state_set_register_request(self, server_port: int) -> asyncio.Condition: # add synchronization primitives to the socket to prevent race condition, also add to api router to prevent problems with 2 routes being added at once
        self.selected_state = RequestRegistration(self, server_port)
        await self.selected_state.state_action()
        
        return asyncio.Condition()
    
    async def state_set_active(self):
        pass
    
    async def state_set_idle(self):
        pass
    
    async def state_set_disconnect(self):
        pass
        
    def get_id(self) -> str:
        return self.id
    
    def get_ip(self) -> str:
        return self.ip
    
    def is_expidited(self) -> bool:
        return self.expidited
        
    
class DeviceState:
    dead_time_interval: int | None = None
    
    def __init__(self, context: Device, **kwargs: dict):
        self.context: Device = context
    
    @abc.abstractmethod
    async def state_action(self) -> Awaitable:
        pass
    
    @abc.abstractmethod
    async def state_failure_action(self, exception: Exception) -> Awaitable:
        pass
    
    @abc.abstractmethod
    def dead_action(self) -> None:
        pass
    
    def cleanup(self) -> None:
        pass
    
    async def run(self) -> Awaitable:
        try:
            await self.state_action()
        except Exception as e:
            await self.state_failure_action(e)
        self.cleanup()
            

class RequestRegistration(DeviceState):
    registration_timeout: int = 30
    def __init__(self, context: Device, server_port: int, selected_router: APIRouter, database_interface: DatabaseInterface):
        super(context)
        self.server_port: int = server_port
        self.database_interface: DatabaseInterface = database_interface
        self.router: APIRouter = selected_router
        
        self.register_event: asyncio.Event = asyncio.Event()
        
    async def state_action(self) -> Awaitable:
        message: RegistrationSchema = RegistrationSchema(server_ip=self.context.get_ip(), server_port=self.server_port).model_dump_json()
        await ScanUDPSocket.send(message, self.context.get_ip())
        
        self.router.add_api_route(f"/register/{self.context.get_ip()}", self.registration_route, methods=["post"])
        
        await asyncio.wait_for(self.register_event, self.registration_timeout)
        
    @override
    async def cleanup(self):
        self.router.routes.remove(f"/register/{self.context.get_ip()}")
        
    async def state_failure_action(self, exception: Exception) -> Awaitable: # this should regress the state to scanned, or something
        raise HTTPException(503, detail=f"Registration of Device {self.context.get_ip()} failed!")
            
    def sync_project_and_preset(self, true_device_id: str, database_device_data: METADATA_OBJECT) -> DeviceRegistrationResponse:
        server_preset_response: Optional[DeviceRegistrationPreset] = None
        server_project_response: Optional[int] = None
        
        database_preset_information: METADATA_OBJECT = self.database_interface.get_preset.execute(QueryByID(id=database_device_data["PresetID"]))

        if database_preset_information:
            server_preset_response = DeviceRegistrationPreset(**database_preset_information)
            
        database_project_information: METADATA_OBJECT = self.database_interface.get_project.execute(QueryByID(id=database_device_data["ProjectID"]))
        
        if database_project_information:
            server_project_response = database_project_information["ProjectID"]
            
        response: DeviceRegistrationResponse = DeviceRegistrationResponse(device_id = true_device_id,
            device_name = database_device_data["DeviceName"],
            project_id = server_project_response,
            preset = server_preset_response,
            reference_time = time.time()
        )
        
        return response
    
    def database_registration(self, database_device_data: METADATA_OBJECT) -> DeviceRegistrationResponse:
        self.database_interface.register_device.execute(RegisterDeviceQuery(
                name=database_device_data["DeviceName"],
                preset_id=database_device_data["PresetID"], # should project and preset be registered to device here? check this
                project_id=database_device_data["ProjectID"],
                device_status="IDLE",
                device_ip=self.request_ip # MUST IMPLEMENT SOME WAY TO GET ACCESS TO ORIGINAL REQUEST! FOR IP
            )
        ) # device name initially set to the ip
        true_device_id: int = self.database_interface.get_device_id_by_ip(QueryByIP(ip_address=self.request_ip))
        
        response = DeviceRegistrationResponse(device_id = true_device_id,
            reference_time=time.time())
        
        return response
        
    async def registration_route(self) -> DeviceRegistrationResponse:
        reported_device_id: str = self.context.get_id()
        
        database_device_data: METADATA_OBJECT = self.database_interface.get_device.execute(QueryByID(id=reported_device_id))
        
        # check to see if the device knows its device id and it doesnt have data in the database. This means the device was unregistered while the device was disconnected
        if reported_device_id and not database_device_data: # this is an edge case. If it becomes necessary ill implement it later
            true_device_id: str = reported_device_id
            
            #await device_unregister.call(source_ip)
            # do something with reregistration here, I dont frickin know
            return
        
        # check to see if the device knows its device id, and it does have an entry in the database
        elif reported_device_id and database_device_data:
            response: DeviceRegistrationResponse = self.sync_project_and_preset(reported_device_id, database_device_data)
        # if the device has never been configured before (if no reported ID)
        else:
            response: DeviceRegistrationResponse = self.database_registration(database_device_data)
            
        self.register_event.notify_all() # only at the end
        
        return response
        

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


