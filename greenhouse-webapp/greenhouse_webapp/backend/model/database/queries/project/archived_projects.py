from pydantic import BaseModel
from model.database.queries.base_query import MetadataListQuery, MetadataObjectQuery
from model.database.queries.query_schemas_shared import QueryByID, QueryByName


class getArchiveProjectID(MetadataObjectQuery[QueryByName]):
    query_str = \
    f"""SELECT ProjectID FROM ArchivedProjects WHERE ProjectName = %(name)s;"""
    
    
class getArchivedProjects(MetadataListQuery[None]):
    query_str = \
    f"""SELECT * FROM ArchivedProjects;"""


class getArchivedProject(MetadataObjectQuery[QueryByID]):
    query_str = \
    f"""SELECT * FROM ArchivedProjects
    WHERE ProjectID = %(id)s;"""