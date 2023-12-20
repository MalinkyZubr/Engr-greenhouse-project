export abstract class PeriodicExecutor {
    private interval: number;

    constructor(interval: number) {
        this.interval = interval;
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