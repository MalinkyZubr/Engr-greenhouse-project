import abc
from typing import Union, Optional, Awaitable, Literal, override, Type
from pydantic import BaseModel
from requests import Response
import requests
from pydantic_core import ValidationError
from fastapi import Request, APIRouter, HTTPException
from socket import socket
from asyncio import Timeout
import asyncio
import time
import sys

from backend.model.devices.udp_schemas import ConnectRequestSchema, RegistrationSchema
from model.database.database_manager import DatabaseInterface, QueryByID, METADATA_OBJECT, RegisterDeviceQuery, QueryByIP
from model.devices.device_manager import ScanUDPSocket
from model.devices.exceptions import *
from controller.device_management.schemas import DeviceRegistrationPreset, DeviceRegistrationResponse

class DeviceState:
    pass


class DeviceStateInterface(abc.ABC):
    @abc.abstractmethod
    async def get(self, route: str, request: BaseModel) -> Awaitable[BaseModel]:
        pass
    
    @abc.abstractmethod
    async def post(self, route: str, request: BaseModel) -> Awaitable[BaseModel]:
        pass
    
    @abc.abstractmethod
    async def delete(self, route: str, request: BaseModel) -> Awaitable[BaseModel]:
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
        
        self.selected_state: DeviceState
        
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
    
    async def state_set_register_request(self, server_port: int) -> Awaitable: # add synchronization primitives to the socket to prevent race condition, also add to api router to prevent problems with 2 routes being added at once
        register_state: RequestRegistrationState = RequestRegistrationState(self, server_port)
        await register_state.setup()
        
        self.selected_state = register_state
        
    async def state_set_active(self):
        self.selected_state = DeviceActiveState(self)
    
    async def state_set_idle(self):
        self.selected_state = DeviceIdleState(self)
    
    async def state_set_disconnect(self):
        self.selected_state = DeviceDisconnectedState(self)
    
    async def state_set_error(self, error: Exception):
        self.selected_state = DeviceErrorState(error)
        
    async def get(self, route: str, request: BaseModel) -> Awaitable[BaseModel]:
        return await self.selected_state.get(route, request)
        
    async def post(self, route: str, request: BaseModel) -> Awaitable[BaseModel]:
        return await self.selected_state.set(route, request)
    
    async def delete(self, route: str, request: BaseModel) -> Awaitable[BaseModel]:
        return await self.selected_state.delete(route, request)
    
    def to_json(self) -> dict[str, Union[str, int, bool]]:
        return {"ip":self.ip, "name":self.name, "id":self.id, "expidited":self.expidited, "state":self.selected_state.to_json().model_dump_json()}
        
    def get_state_name(self) -> str:
        return self.selected_state.__class__.__name__
        
    def get_id(self) -> str:
        return self.id
    
    def get_ip(self) -> str:
        return self.ip
    
    def is_expidited(self) -> bool:
        return self.expidited
        
    
class DeviceState:
    request_interval: int = 3
    request_fail_count: int = 3
    methods = Literal["GET", "PUT", "POST", "DELETE"]
    dead_time_interval: int | None = None
    
    def __init__(self, context: Device, **kwargs: dict):
        self.context: Device = context
        
    def __submit_request(self, ip: str, method: DeviceState.methods, route: str, data: Optional[BaseModel]) -> BaseModel | None:
        match method:
            case "GET":
                response: Response = requests.get(f"{ip}{route}", timeout=DeviceState.request_interval)
            case "PUT":
                response: Response = requests.put(f"{ip}{route}", data=data.model_dump_json(), timeout=DeviceState.request_interval)
            case "POST":
                response: Response = requests.post(f"{ip}{route}", data=data.model_dump_json(), timeout=DeviceState.request_interval)
            case "DELETE":
                response: Response = requests.delete(f"{ip}{route}", data=data.model_dump_json(), timeout=DeviceState.request_interval)

        return response

    async def make_request(self, method: DeviceState.methods, route: str, data: Optional[BaseModel]) -> BaseModel:
        response: Response | None = None
        for _ in range(DeviceState.request_fail_count):
            try:
                self.__submit_request(self.context.get_ip(), method, route, data)
            except Timeout:
                await asyncio.sleep(DeviceState.request_interval)
                continue
        if not response:
            raise HTTPException(500, detail="Request to device failed!")
        
        return response
    
    async def get(self, route: Literal[""], request: BaseModel) -> Awaitable[BaseModel]:
        raise RequestNotImplemented(self.context.get_id(), self.context.get_state_name(), "GET")
    
    async def post(self, route: Literal[""], request: BaseModel) -> Awaitable[BaseModel]:
        raise RequestNotImplemented(self.context.get_id(), self.context.get_state_name(), "POST")
    
    async def delete(self, route: Literal[""], request: BaseModel) -> Awaitable[BaseModel]:
        raise RequestNotImplemented(self.context.get_id(), self.context.get_state_name(), "DELETE")
    
    @abc.abstractmethod
    async def error_handler(self, exception: Exception) -> Awaitable:
        pass
    
    @abc.abstractmethod
    def to_json(self) -> BaseModel:
        pass
    
    def cleanup(self) -> None:
        pass
    
    async def run(self, method: Literal["get", "set", "delete"], request: BaseModel) -> Awaitable:
        try:
            match method:
                case "get":
                    self.get(request)
                case "set":
                    self.set(request)
                case "delete":
                    self.delete(request)
        except Exception as e:
            await self.error_handler(e)
        self.cleanup()
            

class RequestRegistrationState(DeviceState):
    registration_timeout: int = 30
    def __init__(self, context: Device, server_port: int, selected_router: APIRouter, database_interface: DatabaseInterface):
        super(context)
        self.server_port: int = server_port
        self.database_interface: DatabaseInterface = database_interface
        self.router: APIRouter = selected_router
        
        self.register_event: asyncio.Event = asyncio.Event()
        
    async def setup(self) -> Awaitable:
        message: RegistrationSchema = RegistrationSchema(server_ip=self.context.get_ip(), server_port=self.server_port).model_dump_json()
        await ScanUDPSocket.send(message, self.context.get_ip())
        
        self.router.add_api_route(f"/register/{self.context.get_ip()}", self.registration_route, methods=["post"])
        
        await asyncio.wait_for(self.register_event, self.registration_timeout)
        
        self.context.state_set_active()
    
    @override
    async def cleanup(self):
        self.router.routes.remove(f"/register/{self.context.get_ip()}")
        
    async def error_handler(self, exception: Exception) -> Awaitable: # this should regress the state to scanned, or something
        self.context.state_set_error(exception)
        raise HTTPException(503, detail=f"Registration of Device {self.context.get_ip()} failed!") from exception
            
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
    
    def database_registration(self, database_device_data: METADATA_OBJECT, request_ip: str) -> DeviceRegistrationResponse: # if the preset and project are specified, the device should be set to active. otherwise it should be set to idle
        self.database_interface.register_device.execute(RegisterDeviceQuery(
                name=database_device_data["DeviceName"],
                preset_id=database_device_data["PresetID"], # should project and preset be registered to device here? check this
                project_id=database_device_data["ProjectID"],
                device_status="IDLE",
                device_ip=request_ip # MUST IMPLEMENT SOME WAY TO GET ACCESS TO ORIGINAL REQUEST! FOR IP
            )
        ) # device name initially set to the ip
        true_device_id: int = self.database_interface.get_device_id_by_ip(QueryByIP(ip_address=self.request_ip))
        
        response = DeviceRegistrationResponse(device_id = true_device_id,
            reference_time=time.time())
        
        return response
        
    async def registration_route(self, request: Request) -> DeviceRegistrationResponse:
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
            response: DeviceRegistrationResponse = self.database_registration(database_device_data, request.client.host)
            
        self.register_event.notify_all() # only at the end
        
        return response
        

class DeviceActiveState(DeviceState):
    @override
    async def post(self, route: Literal[
            "/reset/network",
            "/time"
            "/configure/id"
            "/configure/preset",
            "/configure/status"
        ], 
    request: BaseModel) -> Awaitable[BaseModel]:
        self.make_request("POST", route, request)
    
    @override
    async def delete(self, route: Literal["/reset/hard"], request: BaseModel) -> Awaitable[BaseModel]:
        self.make_request("DELETE", route, request)

    
class DeviceIdleState(DeviceState):
    pass
        
    
class DeviceDisconnectedState(DeviceState):
    def __init__(self, server_port: int):
        self.server_port: int = server_port
        
    @override
    async def get(self) -> None:
        raise DeviceDisconnectedException(self.context.get_id())
    
    @override
    async def post(self) -> None:
        raise DeviceDisconnectedException(self.context.get_id())
    
    @override
    async def delete(self) -> None:
        raise DeviceDisconnectedException(self.context.get_id())
    
    async def error_handler(self, exception: Exception) -> Awaitable:
        pass

class DeviceErrorState(DeviceState): # needs advance state
    def __init__(self, exception: Exception):
        self.exception: Exception = exception
        
    @override
    async def get(self) -> None:
        raise DeviceErrorException(self.exception)
    
    @override
    async def post(self) -> None:
        raise DeviceErrorException(self.exception)
    
    @override
    async def delete(self) -> None:
        raise DeviceErrorException(self.exception)
    
    async def error_handler(self, exception: Exception) -> Awaitable:
        pass
    
        
class DeviceHaltedState(DeviceState):
    async def post(self, route: Literal["/configure/status"], request: BaseModel) -> Awaitable[BaseModel]:
        self.make_request("POST", route, request)
    
# class DeviceContainer(TaskSubject):
#     def __init__(self):
#         self.devices: dict[Device] = dict()
        
#     def add_device(self, device_id: str, device: Device) -> None:
#         self.devices.set(device_id, device)
        
#     def remove_device(self, device_id: str) -> None:


