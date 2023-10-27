from fastapi import APIRouter

router = APIRouter()

@router.get()
async def list_presets():
    """
    List the current presets loaded into the database
    """
    pass

@router.get()
async def show_preset():
    """
    Retrieve all the features of a specific preset
    """
    pass

@router.post()
async def create_preset():
    """
    Create a preset and load it into the database
    """
    pass

@router.put()
async def update_preset():
    """
    update a preset's fields
    """
    pass

@router.delete()
async def delete_preset():
    """
    delete a specified preset
    """
    pass