from pydantic import BaseModel


class DateQuerySchema(BaseModel): # add input validation here
    start_date: str
    end_date: str