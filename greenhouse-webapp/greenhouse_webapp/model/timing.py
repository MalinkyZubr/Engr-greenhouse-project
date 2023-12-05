from datetime import datetime, timedelta
import matplotlib.dates as dates

MAX_TIME = datetime.fromtimestamp(32536799990).strftime("%y-%m-%d")
MIN_TIME = datetime.fromtimestamp(0).strftime("%y-%m-%d")


def get_today_tomorrow() -> tuple[str, str]:
    """gets the formatted times for the current day, and the next day

    Returns:
        tuple[str, str]: _description_
    """
    today = datetime.today().strftime("%y-%m-%d")
    tomorrow = (datetime.today()+timedelta(1)).strftime("%y-%m-%d")
    return today, tomorrow

def convert_time_formats(m_d_y: str) -> str:
    """convert time format to y-m-d from m-d-y

    Args:
        m_d_y (str): incorrect time formatting

    Returns:
        str: correct time formatting
    """
    return datetime.strftime(datetime.strptime(m_d_y, "%m-%d-%y"), "%y-%m-%d")

def matplot_times(desired_datetime: datetime) -> datetime:
    """convert datetimes into a format acceptable for matplotlib

    Args:
        desired_datetime (datetime): unfromatted time

    Returns:
        datetime: formatted time
    """
    return datetime.strptime(datetime.strftime(desired_datetime, "%m-%d-%Y %H:%M:%S"), "%m-%d-%Y %H:%M:%S")

def get_difference(start_datetime: datetime, end_datetime: datetime) -> str:
    """get the difference between two times

    Args:
        start_datetime (datetime): start time of the interval
        end_datetime (datetime): end time of the interval

    Returns:
        str: formatted time difference between both edges of the interval
    """
    difference_delta: timedelta = end_datetime - start_datetime
    seconds: int = difference_delta.seconds
    minutes: int = seconds / 60
    seconds: int = seconds % 60
    hours: int = minutes / 60
    minutes: int = hours % 60
    
    return f"{hours}:{minutes}:{seconds}"