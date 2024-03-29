from pydantic import BaseModel, Field


class PresetSchema(BaseModel):
    daytime_temp: float = Field(gt=0)
    humidity: float = Field(gt=0, lt=100)
    moisture: float = Field(gt=0, lt=100)
    light_exposure: float = Field(gt=0, lt=100000)