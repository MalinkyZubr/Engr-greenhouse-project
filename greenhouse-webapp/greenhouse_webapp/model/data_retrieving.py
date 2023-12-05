import matplotlib.pyplot as plt
import matplotlib.dates as dates
import os
import asyncio
import time
import csv

from model.timing import matplot_times


LOCAL_DIR = os.path.dirname(os.path.abspath(__file__))
TEMP_IMAGES = os.path.join(LOCAL_DIR, "/temp-images")


class DataRetriever:
    data_mapping = {
        "DeviceID":0,
        "ProjectID":1,
        "DateCollected":2,
        "Temperature":3,
        "Humidity":4,
        "Moisture":5,
        "LightExposure":6,
    }
    def filter_data_project_vis(self, provided_data, data_type, data_interface) -> list[tuple]:
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
            try:
                device_name = data_interface.execute("getDeviceName", device_id)[0][0]
            except IndexError:
                device_name = device_id
            named_device_data[device_name] = zip(*value) # prep the data to be displayed. The zip function flattens the data so that there is one tuple of all x values, and one tuple of all y
        print(named_device_data)
        return named_device_data
    
    def generate_csv(self, provided_data, project_name):
        csv_file_name = f"{LOCAL_DIR}/temp-csv/{project_name}.csv"
        
        with open(csv_file_name, mode='w') as csv_file:
            csv_headers = list(self.data_mapping.keys())
            
            writer = csv.DictWriter(csv_file, fieldnames=csv_headers)
            writer.writeheader()
            
            for datapoint in provided_data:
                writer.writerow({
                    "DeviceID":datapoint[0],
                    "ProjectID":datapoint[1],
                    "DateCollected":datapoint[2],
                    "Temperature":datapoint[3],
                    "Humidity":datapoint[4],
                    "Moisture":datapoint[5],
                    "LightExposure":datapoint[6],
            })
        return csv_file_name
        
    def generate_data_image(self, provided_data, data_type, project_name, data_interface):
        if provided_data:
            filtered_data = self.filter_data_project_vis(provided_data, data_type, data_interface)
        
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
            plt.close()
        else:
            file_name = f"{LOCAL_DIR}/perm-images/no_data.png"
        
        return file_name
    
    async def clean_files(self):
        while True:
            images = [f"{LOCAL_DIR}/temp-images/{image}" for image in os.listdir(f"{LOCAL_DIR}/temp-images")]
            csv = [f"{LOCAL_DIR}/temp-csv/{csv_file}" for csv_file in os.listdir(f"{LOCAL_DIR}/temp-csv")]
            for image in images:
                if time.time() - os.path.getmtime(image) > 1800:
                    os.remove(image)
            for csv_file in csv:
                if time.time() - os.path.getmtime(csv_file) > 1800:
                    os.remove(csv_file)
            await asyncio.sleep(900)
    
    
        

            
        
    