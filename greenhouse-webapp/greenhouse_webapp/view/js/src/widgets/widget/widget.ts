import { AbstractBaseWidgetHTMLController } from "./dynamic/widget_html";
import { BaseStartupFieldParameters } from "./dynamic/widget_html";
import type { FieldParameters } from "./dynamic/field_container";


export type ListParameters = {
    [key: string]: FieldParameters
}

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
    attach_widget_controller(widget_html_controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>): void;
}


export abstract class WidgetModule<HTMLControllerType extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> = AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>> implements Module {
    private widget_html_controller: null | HTMLControllerType = null;

    protected get_widget_html_controller(): HTMLControllerType {
        return this.widget_html_controller ?? function(module: Module) { throw new Error(`widget_html_controller not attached for ${module.constructor.name}`) }(this);
    }

    public attach_widget_controller(widget_html_controller: HTMLControllerType): void {
        this.widget_html_controller = widget_html_controller;
    }

    abstract run(...args: any[]): Promise<void>;
}


export class EmptyModule extends WidgetModule {
    public async run(): Promise<void> {
        throw new EvalError(`The widget ${this.get_widget_html_controller().get_id()} has no modules to execute`)
    }
}


export abstract class WidgetRequestModule<HTMLControllerType extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> = AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>> extends WidgetModule<HTMLControllerType> {
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

    abstract process_response(response: FieldParameters | ListParameters): void;

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

export class BasicUpdaterRequestModule extends WidgetRequestModule {
    public generate_request_body(): object {
        return {
            method: "GET"
        };
    }

    public process_response(response: FieldParameters): void {
        this.get_widget_html_controller().update_dynamic_fields(response);
    }
}

export class WidgetListenerModule<HTMLControllerType extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> = AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>> extends WidgetModule<HTMLControllerType> {
    private html_element: HTMLElement | null = null;
    private dependency: BaseWidget | null = null;
    private listening_element_id: string | undefined;
    private listening_event: string;

    private operation: null | ((controller: HTMLControllerType) => Promise<void>) = null;

    constructor(listen_event: string, listening_element_id?: string) {
        super();
        if(this.get_widget_html_controller().get_node().childElementCount > 1 && !listening_element_id) {
            throw new Error(`The HTMLController ${this.get_widget_html_controller().constructor.name} has more than one field, 
            must select listening_element_id to specify what child to listen for event on`)
        }
        this.listening_element_id = listening_element_id
        this.listening_event = listen_event;

        this.run = this.run.bind(this);
    }

    public attach_widget_controller(widget_html_controller: HTMLControllerType): void {
        super.attach_widget_controller(widget_html_controller);
        if(this.listening_element_id) {
            this.html_element = this.get_widget_html_controller().extract_child(this.listening_element_id);
        }
        else {
            this.html_element = this.get_widget_html_controller().get_node();
        }
        this.html_element.addEventListener(this.listening_event, this.run);
    }

    public async run(...args: any[]): Promise<void> {
        //console.log(this.get_operation());
        if(this.operation) {
            await this.operation(this.get_widget_html_controller());
        }
        if(this.dependency) {
            await this.dependency.run()
        }
    }

    public inject_dependency(dependency: BaseWidget): WidgetListenerModule<HTMLControllerType> {
        this.dependency = dependency;
        return this;
    }

    public inject_operation(operation: (controller: HTMLControllerType) => Promise<void>): WidgetListenerModule<HTMLControllerType> {
        this.operation = operation;
        return this;
    }
}

export class RepetitiveModuleWrapper<HTMLControllerType extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> = AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>> implements Module {
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

    public attach_widget_controller(widget_html_controller: HTMLControllerType): void {
        this.module.attach_widget_controller(widget_html_controller);
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
    private widget_html_controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>;
    private widget_metadata: BaseWidgetMetadata; 
    private widget_core_module: Module = new EmptyModule();
    private listener_modules: Array<WidgetListenerModule> = new Array<WidgetListenerModule>
    private repetetive_modules: Array<RepetitiveModuleWrapper> = new Array<RepetitiveModuleWrapper>();

    constructor(widget_html_object: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>, widget_metadata: BaseWidgetMetadata) {
        this.widget_html_controller = widget_html_object;
        this.widget_metadata = widget_metadata;
        this.widget_core_module.attach_widget_controller(this.widget_html_controller)

        this.create_node(widget_metadata);
    }

    private create_node(widget_metadata: BaseWidgetMetadata) {
        var widget_node: HTMLElement = document.createElement('div');

        widget_metadata.get_widget_parent_node().appendChild(widget_node);
        this.widget_html_controller.assign_widget_node(widget_node);
    }

    public get_name(): string {
        return this.widget_metadata.get_widget_name();
    }

    public get_id(): string {
        return this.widget_html_controller.get_id();
    }

    public get_value(): FieldParameters {
        return this.widget_html_controller.get_value();
    }

    public set_value(parameters: FieldParameters) {
        this.widget_html_controller.update_dynamic_fields(parameters);
    }

    public async run(): Promise<void> {
        await this.widget_core_module.run();
    }

    public add_core_module(new_module: WidgetModule): BaseWidget {
        new_module.attach_widget_controller(this.widget_html_controller);
        this.widget_core_module = new_module;
        
        return this
    }

    public add_listener_module(new_module: WidgetListenerModule): BaseWidget {
        new_module.attach_widget_controller(this.widget_html_controller);
        this.listener_modules.push(new_module);
        
        return this
    }

    public add_module_repetitive(new_module: WidgetModule, interval: number): BaseWidget {
        var repetetive_module: RepetitiveModuleWrapper = new RepetitiveModuleWrapper(new_module, interval);
        repetetive_module.attach_widget_controller(this.widget_html_controller);
        repetetive_module.run();

        this.repetetive_modules.push(repetetive_module);

        return this;
    }

    public kill_module_repetitive() {
        if(this.repetetive_modules.length > 0) {
            for(var repetitive_module of this.repetetive_modules) {
                repetitive_module.kill();
            }
        }
        else {
            throw new Error(`Cannot kill task for ${this.constructor.name}, no task is active`)
        }
    }

    public equals(parameters: FieldParameters): boolean {
        return this.widget_html_controller.equals(parameters);
    }

    public delete_from_dom() {
        this.widget_metadata.get_widget_parent_node()
        .removeChild(
            this.widget_html_controller.get_node()
        );
    }
}