import { AbstractBaseWidgetHTMLController } from "./dynamic/widget_html";
import { BaseStartupFieldParameters } from "./dynamic/widget_html";
import { WidgetReturn } from "./dynamic/widget_return";

export class BasePage {
    private nodes: Array<HTMLElement>;
    private widgets: Map<string, AbstractBaseWidget>;

    public get_widgets(): Map<string, AbstractBaseWidget> {
        return this.widgets;
    }

    public add_widget(widget: AbstractBaseWidget): BasePage {
        this.widgets.set(widget.get_id(), widget);
        return this;
    }
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
    private widget_parent_page: BasePage;

    constructor(widget_name: string, widget_parent: WidgetParent, widget_parent_page: BasePage) {
        this.widget_name = widget_name;
        this.widget_parent = widget_parent.get_parent_node();
        this.widget_parent_page = widget_parent_page;
    }

    public get_widget_parent_node(): HTMLElement {
        return this.widget_parent;
    }

    public get_widget_name(): string {
        return this.widget_name;
    }

    public get_page(): BasePage {
        return this.widget_parent_page;
    }
}

abstract class WidgetModule {
    private widget_html_controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>

    protected get_widget_html_controller(): AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> {
        return this.widget_html_controller;
    }

    public attach_widget_controller(widget_html_controller: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>) {
        this.widget_html_controller = widget_html_controller;
    }

    abstract run(): Promise<void>;
}

abstract class WidgetRequestModule extends WidgetModule {
    protected request_interval: number;
    private request_route: string;

    public constructor(request_route: string, request_interval: number) {
        super()
        this.request_route = request_route;
        this.request_interval = request_interval;
    }

    abstract generate_request_body(): object;

    abstract process_response(): void;

    public async submit_request(): Promise<void> {
        const request_path = `https://${host}:${port}${this.request_route}`
        var dynamic_update_parameters: object = new Object();

        await fetch(request_path, this.generate_request_body())
        .then(res => res.json())
        .then()
    }
}

abstract class RepetitiveWidgetRequestModule extends WidgetRequestModule {
    private run_flag = false;

    private async sleep(): Promise<void> {
        await new Promise(r => setTimeout(r, this.request_interval));
    }

    private async loop(): Promise<void> {
        while(true) {
            await this.submit_request();
            await this.sleep();
        }
    }

    public async run(): Promise<void> {
        if(!this.run_flag) {
            this.run_flag = true;
            await this.loop()
        }
    }
}

class BasicUpdater extends WidgetRequestModule {
    public submit_request() {
        const request_path = `https://${host}:${port}${this.update_route}`
        var dynamic_update_parameters: object = new Object();

        await fetch(request_path, {
            method: "GET"
        })
        .then(res => res.json())
        .then(res => {
            dynamic_update_parameters = res;
        });

        dynamic_update_parameters = await this.update_processing(dynamic_update_parameters);

        this.dynamic_fields.set_field_values(dynamic_update_parameters);
    }
}

class WidgetListenerModule extends WidgetModule {
    private HTMLElement: HTMLElement;
    private listen_event: string;
    private dependency: AbstractBaseWidget;

    constructor(request_route: string, listen_event: string) {
        super(request_route);
        this.listen_event = listen_event;
        this.add_callback(listen_event);
    }

    public process_response() {

    }

    private add_callback(listen_event: string) {
        this.HTMLElement.addEventListener(listen_event, this.submit_request);
    }

    public inject_dependency(dependency: AbstractBaseWidget) {
        this.dependency = dependency;
    }
}

export abstract class AbstractBaseWidget {
    private widget_html: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>;
    private widget_metadata: BaseWidgetMetadata; 
    private widget_modules: Array<WidgetModule>;

    constructor(widget_html_object: AbstractBaseWidgetHTMLController<BaseStartupFieldParameters>, widget_metadata: BaseWidgetMetadata) {
        this.widget_html = widget_html_object;
        this.widget_metadata = widget_metadata;
        this.widget_modules = new Array<WidgetModule>();

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

    public get_widget_name(): string {
        return this.widget_metadata.get_widget_name();
    }

    public async run() {
        this.widget_html.get_update();
    }

    public get_value(): WidgetReturn {
        return this.widget_html.get_value();
    }

    public add_module(new_module: WidgetModule) {
        this.widget_modules.push(new_module);
        new_module.attach_widget_controller(this.widget_html);
    }
}