import matplotlib.pyplot as plt
import matplotlib.dates as dates
import os
import asyncio
import time

from model.timing import matplot_times


LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))
TEMP_IMAGES = os.path.join(LOCAL_DIR, "/temp-images")


class DataVisualizer:
    data_mapping = {
        "DeviceID":0,
        "ProjectID":1,
        "DateCollected":2,
        "Temperature":3,
        "Humidity":4,
        "Moisture":5,
        "LightExposure":6,
        "IRExposure":7,
        "pHLevel":8
    }
    def filter_data_project(self, provided_data, data_type, data_interface) -> list[tuple]:
        devices = dict()
        for datapoint in provided_data:
            data_value = datapoint[self.data_mapping[data_type]]
            device_id = datapoint[self.data_mapping["DeviceID"]]
            timestamp = matplot_times(datapoint[self.data_mapping["DateCollected"]])
            
            try:
                devices[device_id].append((timestamp,data_value))
            except:
                devices[device_id] = []
                devices[device_id].append((timestamp,data_value))
        
        named_device_data = dict()
        
        for device_id, value in devices.items():
            device_name = data_interface.execute("getDeviceName", device_id)[0][0]
            if device_name:
                print(device_name)
                named_device_data[device_name] = zip(*value)
            else:
                print(device_id)
                named_device_data[device_id] = zip(*value) # prep the data to be displayed. The zip function flattens the data so that there is one tuple of all x values, and one tuple of all y
        print(named_device_data)
        return named_device_data
        
    def generate_data_image(self, provided_data, data_type, project_name, data_interface):
        if provided_data:
            filtered_data = self.filter_data_project(provided_data, data_type, data_interface)
        
            file_name = f"{LOCAL_DIR}/temp-images/{project_name}_{data_type}.png"
        
            figure = plt.figure()
            plt.gcf().set_size_inches(10, 6)
            plot = figure.add_subplot()
            
            plt.xlabel("Datetime")
            plt.ylabel(data_type)
            plt.title(f"{data_type} for Project '{project_name}' Over Time")
            plot.xaxis.set_major_formatter(dates.DateFormatter("%m-%d-%Y %H:%M:%S"))
            plt.xticks(rotation=45, ha='right')
            
            for device_name, device_data in filtered_data.items():
                plot.plot(*device_data, "-o", label=device_name)
            plot.legend()
            
            with open(file_name, 'w') as f:
                f.write(" ")
            plt.tight_layout()
            plt.savefig(file_name)
        else:
            file_name = f"{LOCAL_DIR}/perm-images/no_data.png"
        
        return file_name
    
    async def clean_images(self):
        while True:
            images = [f"{TEMP_IMAGES}/{image}" for image in os.listdir(TEMP_IMAGES)]
            for image in images:
                if time.time() - os.path.getmtime(image) > 1800:
                    os.remove(image)
            await asyncio.sleep(900)
    
    
        

            
        
    