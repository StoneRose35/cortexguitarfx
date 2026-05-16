let detectDeviceConsole;
let editorButton;
let ppfxDevice;

const MSG_ABOUT=0;
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
    getAbout();
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
    //const reader = ppfxDevice.readable.getReader();

    //const { value, done } = await reader.read();
    //reader.releaseLock();
    //return value;
    
}

async function readFromPort()
{
    const reader = ppfxDevice.readable.getReader();
    try {
    while(true)
    {
        const { value, done } = await reader.read();
        if (done)
        {
            break;
        }
        if ((value[0] | (value[1] << 8))==MSG_ABOUT)
        {
            detectDeviceConsole.innerText  = new TextDecoder().decode(value.subarray(2));
        }
        else
        {
            console.log("unknown command " + value[0] + " " + value[1] + " received");
        }
    }
    }
    catch (error)
    {

    }
    finally
    {
        reader.releaseLock();
    }
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
                    readFromPort();
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
                    readFromPort();
                }).catch(() =>{
                    detectDeviceConsole.innerText = "Failed to Open VCom Port";
                });
            }).catch(() => {
                detectDeviceConsole.innerText = "No Device Found or selected";
            });
        }
    });


}