from fastapi import APIRouter, Depends

from DBIntRouter import APIDRouter

router = APIDRouter(
    prefix = "/devices",
    tags = ["items"], # this will just assign the tag visually when looking at API docs
    responses = {404: {"description": "Not found"}} # response codes and respective descriptions
    # response errors must be raised with HTTPException
)

@router.get("/")
async def list_devices():
    """
    List registered devices, whether they are connected or disconnected
    """
    pass

@router.get("/{device_id}")
async def get_device(device_id: str):
    """
    get a specific device
    """
    pass

@router.get("/scan")
async def scan_devices():
    """
    Scan for available devices
    """
    pass

@router.post("/scan")
async def register_device():
    """
    register a device to the server
    this should send a UDP resonse to the client telling it to stop sending pings.
    should also remove device from the scan list
    """
    pass