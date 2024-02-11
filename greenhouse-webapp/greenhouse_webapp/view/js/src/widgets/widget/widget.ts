import { AbstractBaseWidgetHTMLController } from "./dynamic/widget_html";
import { BaseStartupFieldParameters } from "./dynamic/widget_html";
import type { FieldParameters } from "./dynamic/field_container";


export class WidgetParent {
    private widget_parent_node: HTMLElement
    
    constructor(widget_parent: string | HTMLElement) {
        if(typeof widget_parent === 'string') {
            this.widget_parent_node = document.getElementById(widget_parent) ?? function() { throw new Error(`id ${widget_parent} not found`) }();
        }
        else {
            this.widget_parent_node = widget_parent;
        }
    }

    public get_parent_node(): HTMLElement {
        return this.widget_parent_node;
    }
}

export class BaseWidgetMetadata {
    private widget_name: string;
    private widget_parent: HTMLElement;
    //private widget_parent_page: BasePage;

    constructor(widget_name: string, widget_parent: WidgetParent) {
        this.widget_name = widget_name;
        this.widget_parent = widget_parent.get_parent_node();
        //this.widget_parent_page = widget_parent_page;
    }

    public get_widget_parent_node(): HTMLElement {
        return this.widget_parent;
    }

    public get_widget_name(): string {
        return this.widget_name;
    }
}

interface Module {
    run(...args: any[]): Promise<void>;
}

export abstract class WidgetModule implements Module {
    private widget_html_controller: null | AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> = null;

    protected get_widget_html_controller(): AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> {
        return this.widget_html_controller ?? function() { throw new Error("widget_html_controller not attached!") }();
    }

    public attach_widget_controller(widget_html_controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>) {
        this.widget_html_controller = widget_html_controller;
    }

    abstract run(...args: any[]): Promise<void>;
}

export class EmptyModule extends WidgetModule {
    public async run(): Promise<void> {
        throw new EvalError(`The widget ${this.get_widget_html_controller().get_id()} has no modules to execute`)
    }
}

export abstract class WidgetRequestModule extends WidgetModule {
    protected request_interval: number;
    private request_route: string;
    public static port: number;
    public static host: string;

    public constructor(request_route: string, request_interval: number) {
        super()
        this.request_route = request_route;
        this.request_interval = request_interval;
    }

    public static set_connection_information(host: string, port: number) {
        WidgetRequestModule.host = host;
        WidgetRequestModule.port = port;
    }

    abstract generate_request_body(): object;

    abstract process_response(response: object): void;

    public async submit_request(): Promise<void> {
        const request_path = `https://${WidgetRequestModule.host}:${WidgetRequestModule.port}${this.request_route}`

        await fetch(request_path, this.generate_request_body())
        .then(res => res.json())
        .then(res => {
            this.process_response(res);
        })
    }

    public async run(): Promise<void> {
        await this.submit_request();
    }
}

// export abstract class WidgetRequestModule extends WidgetModule {
//     protected request_interval: number;
//     private request_route: string;
//     public static port: number;
//     public static host: string;

//     public constructor(request_route: string, request_interval: number, 
//                 request_generator: null | ((widget_request_module: WidgetRequestModule) => FieldParameters),
//                 process_response: null | ((widget_request_module: WidgetRequestModule, response: FieldParameters) => void)) 
//             {
//         super()
        
//         if(request_generator) {
//             this.generate_request_body = request_generator;
//         }
//         if(process_response) {
//             this.process_response = process_response;
//         }

//         this.request_route = request_route;
//         this.request_interval = request_interval;
//     }

//     private generate_request_body: (widget_request_module: WidgetRequestModule) => FieldParameters = 
//         (widget_request_module: WidgetRequestModule) => {
//             return {
//                 method: "GET"
//             };
//         };

//     private process_response: (widget_request_module: WidgetRequestModule, response: FieldParameters) => void = 
//         (widget_request_module: WidgetRequestModule, response: FieldParameters) => { 
//             widget_request_module.get_widget_html_controller().update_dynamic_fields(response) 
//         };

//     public static set_connection_information(host: string, port: number) {
//         WidgetRequestModule.host = host;
//         WidgetRequestModule.port = port;
//     }

//     public async submit_request(): Promise<void> {
//         const request_path = `https://${WidgetRequestModule.host}:${WidgetRequestModule.port}${this.request_route}`

//         await fetch(request_path, this.generate_request_body(this))
//         .then(res => res.json())
//         .then(res => {
//             this.process_response(this, res);
//         })
//     }

//     public async run(): Promise<void> {
//         await this.submit_request();
//     }
// }

export class BasicUpdater extends WidgetRequestModule {
    public generate_request_body(): object {
        return {
            method: "GET"
        };
    }

    public process_response(response: FieldParameters): void {
        this.get_widget_html_controller().update_dynamic_fields(response);
    }
}

export abstract class WidgetListenerModule extends WidgetModule {
    private html_element: HTMLElement;
    private dependency: BaseWidget | null = null;
    private operation: null | ((controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>) => void) = null;

    constructor(listen_event: string, listening_element_id: string) {
        super();
        this.html_element = this.get_widget_html_controller().extract_child(listening_element_id);
        this.add_callback(listen_event);
    }

    private add_callback(listen_event: string) {
        this.html_element.addEventListener(listen_event, this.callback);
    }

    private callback() {
        if(this.dependency) {
            this.dependency.run()
        }
        if(this.operation) {
            this.operation(this.get_widget_html_controller());
        }
    }

    public inject_dependency(dependency: BaseWidget): WidgetListenerModule {
        this.dependency = dependency;
        return this;
    }

    public inject_operation(operation: (controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>) => void) {
        this.operation = operation;
    }
}

export class RepetitiveModuleWrapper implements Module {
    private module: WidgetModule;
    private interval: number;
    private run_flag: boolean = true;

    constructor(module: WidgetModule, interval: number) {
        this.module = module;
        this.interval = interval;
    }

    private async sleep(): Promise<void> {
        await new Promise(r => setTimeout(r, this.interval));
    }

    private async loop(): Promise<void> {
        while(this.run_flag) {
            await this.module.run();
            await this.sleep();
        }
    }

    public async run(): Promise<void> {
        this.run_flag = true;
        await this.loop()
    }

    public kill() {
        this.run_flag = false;
    }
}

export class BaseWidget {
    private widget_html: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>;
    private widget_metadata: BaseWidgetMetadata; 
    private widget_module: Module = new EmptyModule();
    private repetetive_module: Module = new EmptyModule();

    constructor(widget_html_object: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>, widget_metadata: BaseWidgetMetadata) {
        this.widget_html = widget_html_object;
        this.widget_metadata = widget_metadata;

        this.create_node(widget_metadata);
    }

    private create_node(widget_metadata: BaseWidgetMetadata) {
        var widget_node: HTMLElement = document.createElement('div');

        widget_metadata.get_widget_parent_node().appendChild(widget_node);
        this.widget_html.assign_widget_node(widget_node);
    }

    public get_name(): string {
        return this.widget_metadata.get_widget_name();
    }

    public get_id(): string {
        return this.widget_html.get_id();
    }

    public get_value(): FieldParameters {
        return this.widget_html.get_value();
    }

    public add_module(new_module: WidgetModule) {
        new_module.attach_widget_controller(this.widget_html);
        this.widget_module = new_module;
    }

    public async run(): Promise<void> {
        await this.widget_module.run();
    }

    public add_module_repetitive(new_module: WidgetModule, interval: number) {
        this.repetetive_module = new RepetitiveModuleWrapper(new_module, interval);
        this.repetetive_module.run();
    }
}