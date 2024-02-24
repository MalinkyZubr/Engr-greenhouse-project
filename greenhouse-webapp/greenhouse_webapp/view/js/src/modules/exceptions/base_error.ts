export class BaseError<ErrorType extends string> extends Error {
    name: ErrorType
    message: string;
    cause: any;

    constructor(name: ErrorType, message: string, cause: any) {
        super();
        this.name = name;
        this.message = message;
        this.cause = cause;
    }
}


/*
Errors to make:

*/