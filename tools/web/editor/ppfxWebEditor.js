let detectDeviceConsole;
let ppfxDevice;
let fxPrograms;
let effectAPrograms;
let effectBPrograms;
let effectCPrograms;
let commandBfr=[];
let currentCommandLength=0;
let currentCommandNr=0xFFFF;

const MSG_ABOUT=0;
const MSG_PROGRAMS=1;
const MSG_INPUTS_AND_MASTER_VOLUME=3;

const USB_CMD_GET_ABOUT=0;
const USB_CMD_GET_INPUTS_AND_MASTER_VOLUME=3;
const USB_CMD_SET_INPUTS_AND_MASTER_VOLUME=4;
const USB_CMD_GET_PROGRAMS=8;

function init() {
    detectDeviceConsole = document.getElementById("editor-console");
    effectAPrograms = document.getElementById("EffectAPrograms");
    effectBPrograms = document.getElementById("EffectBPrograms");
    effectCPrograms = document.getElementById("EffectCPrograms");
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
    let cmd = new Uint8Array(cmdbfr);
    cmd[0]=USB_CMD_GET_ABOUT;
    cmd[1]=0;
    cmd[2]=4;
    cmd[3]=0;
    const writer = ppfxDevice.writable.getWriter();
    await writer.write(cmd);
    writer.releaseLock();    
}
function getProgramsHandler()
{
    getPrograms();
}

async function getPrograms()
{
    let cmdbfr= new ArrayBuffer(4);
    let cmd = new Uint8Array(cmdbfr);
    cmd[0]=USB_CMD_GET_PROGRAMS;
    cmd[1]=0;
    cmd[2]=4;
    cmd[3]=0;
    const writer = ppfxDevice.writable.getWriter();
    await writer.write(cmd);
    writer.releaseLock();  
}

function processAboutMessage(msg)
{
    detectDeviceConsole.innerText  += String.fromCharCode(...msg.slice(4,msg.length));
}

function processGetPrograms(msg)
{
    let n_programs = msg[4];
    let progCnt=0;
    fxPrograms=new Array(n_programs);
    let idx=5;
    while (idx < msg.length)
    {
        
        let fxProg={ParameterCount: msg[idx]& 0x7F,freezable: false,name: ""};
        if ((msg[idx] & 0x80)>0)
        {
            fxProg.freezable=true;
        }
        let nameArray=[];
        idx++;
        while(msg[idx]!=0 && idx < msg.length)
        {
            nameArray.push(msg[idx++]);
        }
        fxProg.name=String.fromCharCode(...nameArray);
        fxPrograms[progCnt++] = fxProg;
        idx++;
    }
    effectAPrograms.innerHTML="";
    effectBPrograms.innerHTML="";
    effectCPrograms.innerHTML="";
    for (const fxProgram of fxPrograms)
    {
        let fxProgramView=document.createElement("option");
        fxProgramView.innerText = fxProgram.name;
        if (fxProgram.freezable===true)
        {
            fxProgramView.className = "editor-fxprogram-freezable";
        } 
        effectAPrograms.appendChild(fxProgramView);
        fxProgramView=document.createElement("option");
        fxProgramView.innerText = fxProgram.name;
        if (fxProgram.freezable===true)
        {
            fxProgramView.className = "editor-fxprogram-freezable";
        } 
        effectBPrograms.appendChild(fxProgramView);
        fxProgramView=document.createElement("option");
        fxProgramView.innerText = fxProgram.name;
        if (fxProgram.freezable===true)
        {
            fxProgramView.className = "editor-fxprogram-freezable";
        } 
        effectCPrograms.appendChild(fxProgramView);
    }
}

async function readFromPort()
{
    while (true)
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
                if ((value[0] | (value[1] << 8))==MSG_ABOUT && currentCommandLength ===0)
                {
                    currentCommandNr = (value[0] | (value[1] << 8));
                    currentCommandLength = (value[2] | (value[3] << 8));
                    commandBfr = commandBfr.concat([].slice.call(value));
                    console.log("MSG_ABOUT received");
                }
                else if ((value[0] | (value[1] << 8))==MSG_PROGRAMS && currentCommandLength === 0)
                {
                    currentCommandNr = (value[0] | (value[1] << 8));
                    currentCommandLength = (value[2] | (value[3] << 8));
                    commandBfr = commandBfr.concat([].slice.call(value));
                    console.log("MSG_PROGRAMS received");
                }
                else if (currentCommandNr !== 0xFFFF)
                {
                    commandBfr = commandBfr.concat([].slice.call(value));
                }
                if (commandBfr.length === currentCommandLength)
                {
                    switch(currentCommandNr)
                    {
                        case MSG_ABOUT:
                            processAboutMessage(commandBfr);
                            console.log("handled MSG_ABOUT");
                            break;
                        case MSG_PROGRAMS:
                            processGetPrograms(commandBfr);
                            console.log("handled MSG_PROGRAMS");
                            break;
                    }
                    
                    commandBfr=[];
                    currentCommandLength=0;
                    currentCommandNr=0xFFFF;
                }
            }
        }
        catch (error)
        {
            console.log("Error reading USB Port");
            console.log(error);
        }
        finally
        {
            reader.releaseLock();
        }
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