export class DynamicField {
    private value: string;
    private field: HTMLElement;

    public constructor(field: HTMLElement) {
        this.field = field;
    }

    public get_value(): string {
        return this.value;
    }

    public set_value(value: string): void {
        this.value = value;
        this.field.innerText = value;
    }
}

export class StaticField extends DynamicField {
    private is_set: boolean = false;

    public set_value(value: string): void {
        if(!this.is_set) {
            super.set_value(value);
        }
        else {
            throw new Error(`static, can only be set once!`);
        }
    }
}