let detectDeviceConsole;
let mainConsole;
let firmwareUpgradeState;
let fwPromise;
let firmwareData;

const SET_INTERFACE = 11;
const DFU_GETSTATUS = 3;
const DFU_DNLOAD = 1;
const SETUP_REQUEST_DFU_DETACH = 0;
function init() {
    detectDeviceConsole = document.getElementById("fwu-console-1");
    mainConsole = document.getElementById("fwu-console-2");
    firmwareUpgradeState={state: "initial",stateNr: 0};
    const xhttp = new XMLHttpRequest();
    xhttp.open("GET","pipicofx_firmwares.php?fmt=json",true);
    xhttp.onload = function (){
        const jsonresp = JSON.parse(this.responseText);
        jsonresp.sort((a,b) => a["timestamp"] - b["timestamp"]);
        jsonresp.forEach((el) => {
            let fw = document.createElement("option");
            fw.innerText = el["fname"];
            document.getElementById("selectFirmware").appendChild(fw);
        });
    };
    xhttp.send();
}

function fakeDetectDevice()
{
    detectDeviceConsole.innerText = "PiPicoFX, V0.6 built 21.4.2023";
}

function fakeUpdatefirmware()
{
   
    mainConsole.innerText="Requesting Interface 2";
    setTimeout(function() {
        mainConsole.innerText+=" ... Done";
    },500);   
}

async function firmwareUpgradeStep()
{
    let dfuStatus;
    let outData=new ArrayBuffer(0);
    switch (firmwareUpgradeState.state)
    {
        case "initial":
            requestDevice();
            break;
        case "deviceDetected":
            // download appropriate firmware
            const fwRequest = new XMLHttpRequest();
            const fwSelect = document.getElementById("selectFirmware");
            fwPromise =new Promise(function(resolve,reject) {
                fwRequest.open("GET","pipicofx_firmwares.php?dload=" + fwSelect.options[fwSelect.selectedIndex].text);
                fwRequest.responseType = "arraybuffer";
                fwRequest.onload=function() {
                    
                    firmwareData = this.response; 
                    if (firmwareData) {
                        resolve(firmwareData);
                    }
                    else
                    {
                        reject();
                    }
                };
                fwRequest.send();
            });

            mainConsole.innerText += "\nOpening Device in App Mode";
            await firmwareUpgradeState.device.open();
            firmwareUpgradeState.stateNr += 1;
            mainConsole.innerText += "\nSelect configuration 0";
            if (firmwareUpgradeState.device.configuration == null)
            {
                await firmwareUpgradeState.device.selectConfiguration(0);
            }
            firmwareUpgradeState.stateNr += 1;
            mainConsole.innerText += "\nClaiming Interface 2";
            await firmwareUpgradeState.device.claimInterface(2);
            firmwareUpgradeState.stateNr += 1;
            mainConsole.innerText += "\nSetting Interface 2";
            
            await firmwareUpgradeState.device.controlTransferOut({requestType: "standard", 
                recipient: "device",
                request: SET_INTERFACE,
                value: 0,
                index: 2},outData);
            firmwareUpgradeState.stateNr += 1;

            dfuStatus = await getDFUStatus(2);
            firmwareUpgradeState.stateNr += 1;
            mainConsole.innerText += "\n" + JSON.stringify(dfuStatus).replace(/,/g,', ');
            if (dfuStatus.bStatus === 'OK' && dfuStatus.bState === 'APP_IDLE')
            {
                mainConsole.innerText += "\nSending detach Request";
                let resp = await firmwareUpgradeState.device.controlTransferOut({requestType: "class", 
                recipient: "interface",
                request: SETUP_REQUEST_DFU_DETACH,
                value: 100,
                index: 2},outData);
                if (resp.status == "ok")
                {
                    mainConsole.innerText += "\nResetting Device";
                    try {
                        await firmwareUpgradeState.device.reset();

                    } catch (err)
                    {
                        console.log(err);
                    }
                    try {
                        await firmwareUpgradeState.device.releaseInterface(2);

                    } catch (err)
                    {
                        console.log(err);
                    }
                    try {
                        await firmwareUpgradeState.device.close();
                    } catch (err)
                    {
                        console.log(err);
                    }
                    firmwareUpgradeState.state = "inDfuMode";
                    firmwareUpgradeState.stateNr += 1;
                    setTimeout(firmwareUpgradeStep,300);
                } 
                else
                {
                    mainConsole.innerText += '\nDetach Failed';
                }

            }
            break;
        case "inDfuMode":
            mainConsole.innerText += "\nget Device again";
            let dev = await navigator.usb.requestDevice({ filters: [{ vendorId: 0x4A37, productId: 0x35D2 }] });
            firmwareUpgradeState.device = dev;
            mainConsole.innerText += "\nreopen device (in DFU mode)";
            await firmwareUpgradeState.device.open();
            if (firmwareUpgradeState.device.configuration == null)
            {
                await firmwareUpgradeStep.device.selectConfiguration(0);
            }
            if (firmwareUpgradeState.device.configuration.interfaces.length!==1)
            {
                mainConsole.innerText += "\nDevice not in Dfu Mode, stopping";
                break;
            }
            mainConsole.innerText += "\nclaiming interface 0";
            await firmwareUpgradeState.device.claimInterface(0);
            dfuStatus = await getDFUStatus(0);
            mainConsole.innerText += "\n" + JSON.stringify(dfuStatus).replace(/,/g,', ');
            if (dfuStatus.bStatus === 'OK' && dfuStatus.bState === 'IDLE')
            {
                const fwData =  await fwPromise;
                const totalBytes = fwData.byteLength - 16; // TODO_GENERALIZE remove suffix from total size
                let bytesTransferred=0;
                let blockNum=0;
                const TransferSize = 0x40; // TODO_GENERALIZE so far blindly assuming 64 bytes
                let progress=0;
                mainConsole.innerText += "\nStarting Download\n";
                while (bytesTransferred < totalBytes)
                {
                    let bytesToSend;
                    if (totalBytes - bytesTransferred > 64)
                    {
                        bytesToSend = 64;
                    }
                    else
                    {
                        bytesToSend = totalBytes - bytesTransferred;
                    }

                    let dchunk = new Uint8Array(firmwareData,blockNum*64,bytesToSend); 
                    await firmwareUpgradeState.device.controlTransferOut({requestType: "class", 
                    recipient: "interface",
                    request: DFU_DNLOAD,
                    value: blockNum,
                    index: 0},dchunk);
                    while(true)
                    {
                        let dlStatus = await getDFUStatus(0);
                        if (dlStatus.bStatus !== 'OK')
                        {
                            mainConsole.innerText += "\nDownload failed!";
                            throw new Error("Firmware Download failed");
                        }
                        
                        if (dlStatus.bState === 'DNLOAD_IDLE')
                        {
                            break;
                        }
                        await new Promise(r => setTimeout(r,20));
                    }
                    bytesTransferred += bytesToSend;
                    blockNum += 1;
                    if (Math.floor((bytesTransferred/totalBytes)*32.0)> progress)
                    {
                        mainConsole.innerText += "=";
                        progress += 1;
                    }
                }
                await firmwareUpgradeState.device.controlTransferOut({requestType: "class", 
                    recipient: "interface",
                    request: DFU_DNLOAD,
                    value: blockNum,
                    index: 0},outData);
                let dlStatus = await getDFUStatus(0);  
                if (dlStatus.bStatus !== 'OK')
                {
                    mainConsole.innerText += "\nDownload failed!";
                    throw new Error("Firmware Download failed");
                } 
                mainConsole.innerText += "\nDownload succeeded";
                await new Promise(r => setTimeout(r,20));
                dlStatus = await getDFUStatus(0);  
                while (dlStatus.bState !== 'MANIFEST_WAIT_RESET')
                {
                    await new Promise(r => setTimeout(r,20));
                    dlStatus = await getDFUStatus(0); 
                }
                mainConsole.innerText += "\nFirmware updated successfully";
                try 
                {
                    await firmwareUpgradeState.device.reset();
                } catch (err)
                {
                    console.log(err);
                }
            }
            break;

    }
}

function getDFUStatus(interfaceNr)
{
    let p = new Promise(function(resolve,reject){
        if (firmwareUpgradeState.stateNr > 2)
        {
            firmwareUpgradeState.device.controlTransferIn({requestType: "class", 
                recipient: "interface",
                request: DFU_GETSTATUS,
                value: 0,
                index: interfaceNr},6).then(
                    (res) => {
                        if (res.status == "ok") // usb transfer ok, can still mean that the dfu state isn't
                        {
                            resolve({
                            bStatus: resolveDFUStatus(res.data.getUint8(0)),
                            bwPollTimeout: res.data.getUint8(1)/1 + (res.data.getUint8(2) << 8) + (res.data.getUint8(3) << 16),
                            bState: resolveDFUState(res.data.getUint8(4)),
                            iString: res.data.getUint8(5)});
                        }
                        else
                        {
                            reject("usb status nok");
                        }
                    },
                    (err) => {
                        console.log(err);
                        reject("promise rejected");
                    }
                );
        }
        else
        {
            reject("wrong dfu update state nr");
        }

    });
    return p;
}

function resolveDFUStatus(statusInt)
{
    switch(statusInt)
    {
        case 0:
            return "OK";
        case 1:
            return "ERR_TARGET";
        case 2:
            return "ERR_FILE";
        case 3:
            return "ERR_WRITE";
        case 4:
            return "ERR_ERASE";
        case 5:
            return "ERR_CHECK_ERASE";
        case 6:
            return "ERR_PROG";
        case 7:
            return "ERR_VERIFY";
        case 8:
            return "ERR_ADDRESS";
        case 9:
            return "ERR_NOTDONE";
        case 10:
            return "ERR_FIRMWARE";
        case 11:
            return "ERR_VENDOR";
        case 12:
            return "ERR_USBR";
        case 13:
            return "ERR_POR";
        case 14:
            return "ERR_UNKNOWN";
        case 15:
            return "ERR_STALLEDPKT";
        default:
            return "UNKNOWN STATUS " + statusInt;
    }
}

function resolveDFUState(stateInt)
{
    /*
    #define USB_DFU_APP_IDLE 0
#define USB_DFU_APP_DETACH 1
#define USB_DFU_IDLE 2
#define USB_DFU_DNLOAD_SYNC 3
#define USB_DFU_DNBUSY 4
#define USB_DFU_DNLOAD_IDLE 5
#define USB_DFU_MANIFEST_SYNC 6 
#define USB_DFU_MANIFEST 7
#define USB_DFU_MANIFEST_WAIT_RESET 8
#define USB_DFU_UPLOAD_IDLE 9
#define USB_DFU_ERROR 10*/
    switch(stateInt)
    {
        case 0:
            return "APP_IDLE";
        case 1:
            return "APP_DETACH";
        case 2:
            return "IDLE";
        case 3:
            return "DNLOAD_SYNC";
        case 4:
            return "DNBUSY";
        case 5:
            return "DNLOAD_IDLE";
        case 6:
            return "MANIFEST_SYNC";
        case 7:
            return "MANIFEST";
        case 8:
            return "MANIFEST_WAIT_RESET";
        case 9:
            return "UPLOAD_IDLE";
        case 10:
            return "ERROR";
        default: 
            return "UNKNOWN DFU STATE " + stateInt;
    }
}

function requestDevice()
{
    detectDeviceConsole.innerText = "";
    navigator.usb
    .requestDevice({ filters: [{ vendorId: 0x4A37, productId: 0x35D2 }] })
    .then((usbDevice) => {
        detectDeviceConsole.innerText = "Got Device";
        firmwareUpgradeState.state = "deviceDetected";
        firmwareUpgradeState.stateNr = 1;
        firmwareUpgradeState.device = usbDevice;
        mainConsole.innerText = "\nPiPicoFX detected";
  })
    .catch(() => {
        detectDeviceConsole.innerText = "No Device Present";
  });
}

