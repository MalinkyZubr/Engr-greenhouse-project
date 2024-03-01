class getArchiveProjectID(DatabaseQuery):
    return_type=ReturnTypes.SINGLE_ELEMENT(0)
    query_str = \
    f"""SELECT ProjectID FROM ArchivedProjects WHERE ProjectName = %s;"""
    def query(self, cursor, project_name):
        return cursor.execute(self.query_str, (project_name,))
    
    
class getArchivedProjects(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM ArchivedProjects;"""
    def query(self, cursor):
        return cursor.execute(self.query_str)


class getArchivedProject(DatabaseQuery):
    query_str = \
    f"""SELECT * FROM ArchivedProjects
    WHERE ProjectID = %s;"""
    def query(self, cursor, project_id):
        return cursor.execute(self.query_str, (project_id,))