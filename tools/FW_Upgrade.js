let detectDeviceConsole;
let mainConsole;
let firmwareUpgradeState;

const SET_INTERFACE = 11;
const DFU_GETSTATUS = 3;
function init() {
    detectDeviceConsole = document.getElementById("fwu-console-1");
    mainConsole = document.getElementById("fwu-console-2");
    firmwareUpgradeState={state: "initial",stateNr: 0};
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

function firmwareUpgradeStep()
{
    switch (firmwareUpgradeState.state)
    {
        case "initial":
            requestDevice();
            break;
        case "deviceDetected":
            firmwareUpgradeState.device.open().then(() => {
                firmwareUpgradeState.state = "deviceOpened";
                mainConsole.innerText += "\ndevice opened";
                firmwareUpgradeStep();
            }, (err) => {
                console.log(err);
            });
            break;
        case "deviceOpened":
            if (firmwareUpgradeState.device.configuration == null)
            {
                firmwareUpgradeState.device.selectConfiguration(0).then(
                    () => {
                        firmwareUpgradeState.state = "deviceConfigured";
                        mainConsole.innerText += "\nDevice configured";
                        firmwareUpgradeStep();
                    },
                    (err) => {
                        console.log(err);
                    }
                )
            }
            else
            {
                firmwareUpgradeState.state = "deviceConfigured";
                mainConsole.innerText += "\nDevice configured";
                firmwareUpgradeStep();
            }
            break;
        case "deviceConfigured":
            firmwareUpgradeState.device.claimInterface(2).then(() => 
                {
                    firmwareUpgradeState.state = "interfaceClaimed";
                    mainConsole.innerText += "\ndfu interface (2) claimed";
                    firmwareUpgradeState.stateNr = 2;
                    firmwareUpgradeStep();
                },(err) =>
                {
                    console.log(err);
                });
            break;
        case "interfaceClaimed":
            let outData=new ArrayBuffer(0);
            firmwareUpgradeState.device.controlTransferOut({requestType: "standard", 
                recipient: "device",
                request: SET_INTERFACE,
                value: 0,
                index: 2},outData).then(
                (resp) => {
                    if (resp.status == "ok")
                    {
                        firmwareUpgradeState.state = "interfaceSet";
                        mainConsole.innerText += "\nInterface 2 set";
                        firmwareUpgradeState.stateNr = 3;
                        firmwareUpgradeStep();
                    }
                    else
                    {
                        mainConsole.innerText += "\ncould not set interface 2 (USB NOK)";
                    }
                },
                (err) => {
                    console.log(err);
                    mainConsole.innerText += "\ncould not set interface 2 (promise failed)";
                }
            );
            break;
        case "interfaceSet":
            getDFUStatus(2).then((res) => {
                mainConsole.innerText += "\n" + JSON.stringify(res).replace(/,/g,', ');
            });
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
        mainConsole.innerText += "\nPiPicoFX detected";
  })
    .catch(() => {
        detectDeviceConsole.innerText = "No Device Present";
  });
}