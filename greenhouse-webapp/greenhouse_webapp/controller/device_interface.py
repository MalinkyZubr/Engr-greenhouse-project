from fastapi import APIRouter
from DBIntRouter import APIDRouter

router = APIDRouter()
"""
this file interacts directly with the devices on the entwork
"""
@router.get()
async def list_devices():
    """
    List registered devices, whether they are connected or disconnected
    """
    pass

@router.get()
async def get_device():
    """
    get a specific device
    """
    pass

@router.get()
async def scan_devices():
    """
    Scan for available devices
    """
    pass