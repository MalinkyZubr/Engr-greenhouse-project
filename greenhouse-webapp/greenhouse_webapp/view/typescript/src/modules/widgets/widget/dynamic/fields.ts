import { HTMLControllerError } from "../../../exceptions/module_errors";

export class DynamicField {
    private value: string = "";
    private field: HTMLElement;

    public constructor(field: HTMLElement) {
        this.field = field;
    }

    public get_value(): string {
        return this.value;
    }

    public set_value(value: string): void {
        this.value = value;
        this.field.innerHTML = value;
    }
}

export class StaticField extends DynamicField {
    private is_set: boolean = false;

    public set_value(value: string): void {
        if(!this.is_set) {
            super.set_value(value);
            this.is_set = true;
        }
        else {
            throw new HTMLControllerError("FIELDS_ERROR", 
                "Static fields may only be set once", this);
        }
    }
}