export abstract class PeriodicExecutor {
    private interval: number;

    constructor() {
        this.interval = 5000;
    }

    abstract call_methods(): Promise<void>;

    async run(): Promise<void> {
        while(true) {
            await this.call_methods();
            await new Promise(function(resolve) {
                setTimeout(resolve, this.interval)
            });
        }
    }
}