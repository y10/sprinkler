import { Http } from './services/http.prod'

const $ = (id) => document.getElementById(id);

async function getwifinetworks() {
    const sr = await Http.json("GET", "scan", null, 15000);
    $('ssid-list').innerHTML = sr
        .sort((a, b) => { return b.q - a.q })
        .map(e => "<option>" + e.ssid + "</option>")
        .join("");
}

async function getinfo() {
    const ir = await Http.json("GET", "info", null, 5000);
    $('chip').value = ir.chip;
    $('name').value = ir.name;
}

async function bootstrap() {
    let i = 0;
    while (i < 3) {
        try {
            await getinfo();
            let n = 0;
            while (n < 3) {
                try {
                    await getwifinetworks();
                    return;
                } catch (error) {
                    n++;
                }
            }
            break;
        } catch (error) {
            i++;
        }
    }
}

bootstrap();