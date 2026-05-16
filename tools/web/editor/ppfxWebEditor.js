let detectDeviceConsole;
let editorButton;
let ppfxDevice;

function init() {
    detectDeviceConsole = document.getElementById("editor-console");
    editorButton = document.getElementById("editor-button");
}


async function openCdcInterface(device)
{
    await device.open();

    navigator.serial.getPorts().then((ports) => {
        ports.forEach( p => {
            detectDeviceConsole.innerText += p.getInfo();
        });
    });
    return device;
}

function aboutHandler()
{
    getAbout().then( (res) => {
        detectDeviceConsole.innerText  = new TextDecoder().decode(res);
    });
}

async function getAbout()
{
    
    let cmdbfr= new ArrayBuffer(4);
    let cmd0 = new Uint8Array(cmdbfr);
    cmd0[0]=0;
    cmd0[1]=0;
    cmd0[2]=4;
    cmd0[3]=0;
    const writer = ppfxDevice.writable.getWriter();
    await writer.write(cmd0);
    writer.releaseLock();
    const reader = ppfxDevice.readable.getReader();

    const { value, done } = await reader.read();
    reader.releaseLock();
    return value;
    
}

function requestDevice()
{
    let devFound=false;
    navigator.serial.getPorts().then( (ports) => {
        ports.forEach(p => {
            const portinfo = p.getInfo();
            if (portinfo.usbVendorId == 0x4A37 && portinfo.usbProductId == 0x35D2)
            {
                devFound = true;
                p.open({baudRate: 115200}).then(() => {
                    detectDeviceConsole.innerText = "PiPicoFX VCom Port Opened";
                    ppfxDevice = p;
                });

            }
        });
        if (devFound === false)
        {
            navigator.serial.requestPort({ filters: [{ usbVendorId: 0x4A37,usbProductId: 0x35D2 }] }).then((p) =>
            {
                p.open({baudRate: 115200}).then(() => {
                    detectDeviceConsole.innerText = "PiPicoFX VCom Port Opened";
                    ppfxDevice = p;
                }).catch(() =>{
                    detectDeviceConsole.innerText = "Failed to Open VCom Port";
                });
            }).catch(() => {
                detectDeviceConsole.innerText = "No Device Found or selected";
            });
        }
    });


}