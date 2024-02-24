import { BaseError } from "./base_error";


type DomErrorCodes = "ELEMENT_NOT_FOUND_ERROR" | "FORMAT_ERROR"
type HTMLControllerErrorCodes = "HTML_TEMPLATE_ERROR" | "FIELDS_ERROR"
type WidgetErrorCodes = "IMPROPER_CONFIGURATION_ERROR" | "NONEXISTANT_CONFIGURATION_ERROR" | "CHILD_ERROR"


export class DomError extends BaseError<DomErrorCodes> {}
export class HTMLControllerError extends BaseError<HTMLControllerErrorCodes> {}
export class WidgetError extends BaseError<WidgetErrorCodes> {}