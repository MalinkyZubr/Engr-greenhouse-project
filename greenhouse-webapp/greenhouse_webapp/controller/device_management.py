from fastapi import APIRouter

router = APIRouter()

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