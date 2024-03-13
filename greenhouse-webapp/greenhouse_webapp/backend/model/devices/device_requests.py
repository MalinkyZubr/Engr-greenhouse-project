from abc import abstractmethod, ABC
from typing import Optional, Type, Literal
from pydantic import BaseModel
from fastapi import Response, HTTPException
from asyncio import Timeout
import asyncio
import requests

from controller.device_management.schemas import DeviceConfigurationSchema, DeviceRegistrationPreset


ATTEMPT_INTERVAL: int = 3
ATTEMPT_COUNT: int = 3
METHODS = Literal["GET", "PUT", "POST", "DELETE"]


def submit_request(ip: str, method: METHODS, route: str, data: Optional[BaseModel]) -> BaseModel | None:
    match method:
        case "GET":
            response: Response = requests.get(f"{ip}{route}", timeout=ATTEMPT_INTERVAL)
        case "PUT":
            response: Response = requests.put(f"{ip}{route}", data=data.model_dump_json(), timeout=ATTEMPT_INTERVAL)
        case "POST":
            response: Response = requests.post(f"{ip}{route}", data=data.model_dump_json(), timeout=ATTEMPT_INTERVAL)
        case "DELETE":
            response: Response = requests.delete(f"{ip}{route}", data=data.model_dump_json(), timeout=ATTEMPT_INTERVAL)

    return response


async def request_device(ip: str, method: METHODS, route: str, data: Optional[BaseModel]) -> BaseModel:
    response: Response | None = None
    for _ in range(ATTEMPT_COUNT):
        try:
            submit_request(ip, method, route, data)
        except Timeout:
            await asyncio.sleep(ATTEMPT_INTERVAL)
            continue
    if not response:
        raise HTTPException(500, detail="Request to device failed!")
    
    return response