from DBIntRouter import APIDRouter

router = APIDRouter()

@router.post()
async def create_project():
    """
    Create a new project and put it into the database
    """
    pass

@router.put()
async def update_project():
    """
    Update a project definition
    """
    pass

@router.get()
async def list_projects():
    """
    List currently available projects
    """
    pass

@router.get()
async def get_project():
    """
    get a specified project
    """
    pass

@router.patch()
async def archive_project():
    """
    archive a project so that it is inactive, but can still be used later
    """
    pass

@router.delete()
async def delete_project():
    """
    permanently delete a project from the database
    """
    pass