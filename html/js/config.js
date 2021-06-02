export const FIRMWARE_URL = "http://ota.voights.net/sprinkler.bin";
export const Version = {
    major:     1,
    minor:     0,
    release:   0,
    build:     11,
    toString(){
        return "1.0.0.1"
    },
    toDecimal(){
        return 100 + (1 * 0.001);
    }
}