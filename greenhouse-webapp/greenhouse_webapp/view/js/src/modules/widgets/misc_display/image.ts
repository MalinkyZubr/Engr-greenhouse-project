import { AbstractBaseWidgetHTMLController, BaseStartupFieldParameters } from "../widget/dynamic/widget_html";
import { Requester } from "../widget/widget";


export class ImageStartupFieldParameters extends BaseStartupFieldParameters {
    private alt_text: string;

    constructor(element_id: string, alt_text: string) {
        super(alt_text);
        this.alt_text = alt_text;
    }
}


export class ImageWidgetHTMLController extends AbstractBaseWidgetHTMLController<BaseStartupFieldParameters> {
    public html_template_generator(): string {
        return `<img id="{{ element_id }} src="" alt="{{ alt_text }}">`;
    }

    public get_node(): HTMLImageElement {
        return super.get_node() as HTMLImageElement;
    }
}


export abstract class WidgetImageRequestModule extends Requester<ImageWidgetHTMLController> {
    abstract generate_request_body(): object;

    public process_response(response: string): void {
        this.get_widget_html_controller().get_node().src = response;
    }

    public async submit_request(): Promise<void> {
        const request_path = this.generate_request_path();

        await fetch(request_path, this.generate_request_body())
        .then(res => res.blob())
        .then(blob => {
            const image_url: string = URL.createObjectURL(blob);
            this.process_response(image_url)
        })
    }
}