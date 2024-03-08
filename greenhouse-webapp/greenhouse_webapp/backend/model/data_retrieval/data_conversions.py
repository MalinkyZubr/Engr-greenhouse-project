from model.database.database_manager import DATA_SCHEMA, get_device_name
from model.database.queries.query_schemas_shared import QueryByID
import csv
import datetime
import os
import matplotlib.pyplot as plt
from matplotlib.dates import dates
import matplotlib


LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))
TEMP_IMAGES = os.path.join(LOCAL_DIR, "/temp-images")
TEMP_CSV = os.path.join(LOCAL_DIR, "/temp-csv")


def generate_file_name(identifier: str, extension: str) -> str:
    current_time = datetime.datetime.now().strftime("%m-%D-%Y_%H-%M-%S")
    filename = f"{identifier}_{current_time}.{extension}"
    
    return filename

def convert_to_csv(identifier: str, data: DATA_SCHEMA) -> str:
    filename = generate_file_name(identifier, "csv")
    with open(f"{TEMP_CSV}/{filename}") as f:
        writer = csv.writer(f)
        writer.writerow(data.keys())
        writer.writerows(zip(*data.values()))
        
    return filename

def get_devicewise_data(data: DATA_SCHEMA, column_name: str) -> dict[str, list[tuple[str, str]]]:
    device_wise_data: dict[str, tuple[list]] = dict()
    
    device_ids = set(data["DeviceID"])
    device_names = {device_id:get_device_name.execute(QueryByID(id=device_id)) for device_id in device_ids}
    
    for index, datapoint in enumerate(data[column_name]):
        datapoint_device_id: str = data["DeviceID"][index]
        datetime_collected: str = data["DateCollected"][index]
        
        device_name: str = device_names[datapoint_device_id]
        if device_name not in device_wise_data:
            device_wise_data[device_name] = []
        else:
            device_wise_data[device_name].append((datetime_collected, datapoint))
            
    return device_wise_data


def generate_matplot_plot(identifier: str, filename: str, column_name: str, date_range: tuple[datetime.datetime, datetime.datetime], data: dict[str, list[tuple[str, str]]]):
    figure: plt.Figure = plt.figure()
    plt.gcf().set_size_inches(10, 6)
    plot: matplotlib.axes._subplots.AxesSubplot = figure.add_subplot()
    
    plt.xlabel("Date & Time Collected")
    plt.ylabel(column_name)
    plt.title(f"{column_name} for Project '{identifier}' Over {date_range}")
    plot.xaxis.set_major_formatter(dates.DateFormatter("%m-%d-%Y %H:%M:%S"))
    plt.xticks(rotation=45, ha='right')
    
    for device_name, device_data in data.items():
        plot.plot(*device_data, "-o", label=device_name)
    plot.legend()
    
    with open(f"{TEMP_IMAGES}/{filename}", 'w') as f:
        f.write("")
    plt.tight_layout()
    plt.savefig(filename)
    plt.close()
            

def convert_to_chart(data: DATA_SCHEMA) -> str:
    date_range = (min(data["DateCollected"]), max(data["DateCollected"]))
    
    identifier = data["ProjectID"][0]
    column_name = tuple(filter(lambda term: (term not in ["ProjectID", "DeviceID", "DateCollected"])), data.keys())[0]
    filename = generate_file_name(identifier, "png")
    device_wise_data = get_devicewise_data(data, column_name)
    
    generate_matplot_plot(identifier, filename, column_name, date_range, device_wise_data)
    
    return filename
    
