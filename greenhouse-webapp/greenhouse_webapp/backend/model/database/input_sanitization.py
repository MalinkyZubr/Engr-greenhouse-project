from pydantic import BaseModel


QUERY_BLACK_LIST = ["--", ";", "/*", "*/", "@@", "@",
                "char", "nchar", "varchar", "nvarchar",
                "alter", "begin", "cast", "create", "cursor",
                "declare", "delete", "drop", "end", "exec",
                "execute", "fetch", "insert", "kill", "open",
                "sys", "sysobjects", "syscolumns",
                "table", "update"]

SQL_BANNED_CHARS = [
    "'",  
    "-",  
    "#",  
    ";",  
    "&",  
    "|",  
    "$",  
    "*",  
    "%",  
    "_",  
    "\\"
]


def check_word(word: str):
    if word in QUERY_BLACK_LIST:
        raise Exception(f"Invalid character {word} detected")
    for character in SQL_BANNED_CHARS:
        if character == word:
            raise Exception(f"Invalid character {character} detected")        
        
def check_query(query_parameters: BaseModel) -> None:
    for query_parameter in query_parameters.model_dump().values():
        check_word(query_parameter)   
        