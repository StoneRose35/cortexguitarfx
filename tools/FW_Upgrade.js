let detectDeviceConsole;
let mainConsole;
let firmwareUpgradeState;
let fwPromise;
let firmwareData;
let firmwareUpdateButton;
let firmwares_json;
let configurationParserCurrentInterface;
const SET_INTERFACE = 11;
const GET_DESCRIPTOR = 6;
const GET_INTERFACE = 10;
const DFU_GETSTATUS = 3;
const DFU_DNLOAD = 1;
const SETUP_REQUEST_DFU_DETACH = 0;


const DESCRIPTOR_DEVICE = 1;
const DESCRIPTOR_CONFIGURATION = 2;
const DESCRIPTOR_STRING = 3;
const DESCRIPTOR_INTERFACE = 4;
const DESCRIPTOR_ENDPOINT = 5;
const DESCRIPTOR_DEVICE_QUALIFIER = 6;
const DESCRIPTOR_OTHER_SPEED_CONFIGURATION = 7;
const DESCRIPTOR_INTERFACE_POWER = 8;
const DESCRIPTOR_DFU_INTERFACE = 33;

function populateFirmwaresCombobox(filterfct)
{
    document.getElementById("selectFirmware").innerHTML = "";
    firmwares_json.sort((a,b) => a["timestamp"] - b["timestamp"]);
    firmwares_json.forEach((el) => {
        var filterres = filterfct(el["type"]);
        if(filterres)
        {
            let fw = document.createElement("option");
            fw.innerText = el["fname"];
            fw.className = el["type"];
            document.getElementById("selectFirmware").appendChild(fw);
        }
    });
}

function init() {
    detectDeviceConsole = document.getElementById("fwu-console-1");
    mainConsole = document.getElementById("fwu-console-2");
    firmwareUpdateButton = document.getElementById("fwu-button");
    firmwareUpgradeState={state: "initial",stateNr: 0};
    const xhttp = new XMLHttpRequest();
    xhttp.open("GET","pipicofx_firmwares.php?fmt=json",true);
    xhttp.onload = function (){
        const jsonresp = JSON.parse(this.responseText);
        firmwares_json = jsonresp;
        populateFirmwaresCombobox(() => true);
    };
    xhttp.send();
}

async function updateFirmwareHandler()
{
    firmwareUpdateButton.disabled=true;
    try {
        if (firmwareUpgradeState.state === "deviceDetectedPPFX")
        {
            firmwareUpgradeStepPPFX();

        }
        else
        {
            firmwareUpgradeStepSTM32();
        }
        firmwareUpdateButton.disabled = false;
    }
    catch {
        firmwareUpdateButton.disabled = false;
    }
}

async function firmwareUpgradeStepPPFX()
{
    let dfuStatus;
    let outData=new ArrayBuffer(0);
    switch (firmwareUpgradeState.state)
    {
        case "initial":
            requestDevice();
            break;
        case "deviceDetectedPPFX":
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
                    setTimeout(firmwareUpgradeStepPPFX,300);
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
                await firmwareUpgradeStepPPFX.device.selectConfiguration(0);
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
                            reject(res.status);
                        }
                    },
                    (err) => {
                        console.log(err);
                        reject("USBPipeError");
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

function getConfigurationDescriptor(size)
{
    let p = new Promise (function(resolve,reject){
        let descriptorHeader =  firmwareUpgradeState.device.controlTransferIn({
                requestType: "standard",
                recipient: "device",
                request: GET_DESCRIPTOR,
                value: (DESCRIPTOR_CONFIGURATION << 8) | 0x0,
                index: 0},size).then( transferResult => {
                    if (transferResult.status == "ok") {
                        var configurationDescriptor = {
                            bLength: 0,
                            bDescriptorType: "",
                            bcdUSB: "",
                            bDeviceClass: 0,
                            bDeviceProtocol: 0,
                            bMaxPacketSize0: 0,
                            bNumConfigurations: 0,
                            bReserved: 0,
                            interfaces: []
                        }
                        var pointer = 0;
                        parseConfigurationDescriptor(transferResult.data,pointer,configurationDescriptor);
                        resolve(configurationDescriptor);
                    }
                    else
                    {
                        reject("Could not get Configuration Descriptor");
                    }
                });
    });
    return p;
}

function parseConfigurationDescriptor(arraydata,pointer,result)
{
    if (arraydata.getUint8(pointer+1)==DESCRIPTOR_CONFIGURATION)
    {
        parseConfigurationDescriptorHeader(arraydata,pointer,result);
        if (pointer + arraydata.getUint8(pointer) < arraydata.byteLength)
        {
            parseConfigurationDescriptor(arraydata,pointer+arraydata.getUint8(pointer),result);
        }
    }
    else if (arraydata.getUint8(pointer+1)==DESCRIPTOR_INTERFACE)
    {
        configurationParserCurrentInterface = parseInterfaceDescriptor(arraydata,pointer,result);
        if (pointer + arraydata.getUint8(pointer) < arraydata.byteLength)
        {
            parseConfigurationDescriptor(arraydata,pointer+arraydata.getUint8(pointer),result);
        }
    }
    else if (arraydata.getUint8(pointer+1)==DESCRIPTOR_ENDPOINT)
    {
        parseEndpointDescriptor(arraydata,pointer,result,configurationParserCurrentInterface);
        if (pointer + arraydata.getUint8(pointer) < arraydata.byteLength)
        {
            parseConfigurationDescriptor(arraydata,pointer+arraydata.getUint8(pointer),result);
        }
    }
    else if (arraydata.getUint8(pointer+1)==DESCRIPTOR_DFU_INTERFACE)
    {
        parseDfuInterfaceDescriptor(arraydata,pointer,result,configurationParserCurrentInterface);
        if (pointer + arraydata.getUint8(pointer) < arraydata.byteLength)
        {
            parseConfigurationDescriptor(arraydata,pointer+arraydata.getUint8(pointer),result);
        }
    }
    else // everything else in unknown, jump by bLength of the current descriptor
    {
        if (pointer + arraydata.getUint8(pointer) < arraydata.byteLength)
        {
            parseConfigurationDescriptor(arraydata,pointer+arraydata.getUint8(pointer),result);
        }
    }
}

function parseConfigurationDescriptorHeader(arraydata,pointer,result)
{
    result.bLength = arraydata.getUint8(pointer);
    result.bDescriptorType = arraydata.getUint8(pointer+1);
    result.wTotalLength = arraydata.getUint16(pointer+2,true);
    result.bNumInterfaces = arraydata.getUint8(pointer+4);
    result.interfaces = Array.apply(null,Array(result.bNumInterfaces)).map(() => {});
    result.bConfigurationValue = arraydata.getUint8(pointer+5);
    result.iConfiguration = arraydata.getUint8(pointer+6);
}

function parseInterfaceDescriptor(arraydata,pointer,result)
{

    var interfacenr = arraydata.getUint8(pointer+2)
    result.interfaces[interfacenr]={};
    result.interfaces[interfacenr].bLength = arraydata.getUint8(pointer);
    result.interfaces[interfacenr].bDescriptorType = arraydata.getUint8(pointer+1);
    result.interfaces[interfacenr].bInterfaceNumber = arraydata.getUint8(pointer+2);
    result.interfaces[interfacenr].bAlternateSetting = arraydata.getUint8(pointer+3);
    result.interfaces[interfacenr].bNumEndpoints = arraydata.getUint8(pointer + 4);
    result.interfaces[interfacenr].endpoints=Array.apply(null,Array(result.interfaces[interfacenr].bNumEndpoints)).map(() => {});
    result.interfaces[interfacenr].bInterfaceClass =arraydata.getUint8(pointer + 5);
    result.interfaces[interfacenr].bInterfaceSubClass = arraydata.getUint8(pointer + 6);
    result.interfaces[interfacenr].bInterfaceProtocol  = arraydata.getUint8(pointer + 7);
    result.interfaces[interfacenr].iInterface = arraydata.getUint8(pointer+8);
    return interfacenr;
}

function parseEndpointDescriptor(arraydata,pointer,result,interfacenr)
{
    var endpointNr = arraydata.getUint8(pointer+2) & 0xF;
    result.interfaces[interfacenr].bEndpointAddress = arraydata.getUint8(pointer+2);
    result.interfaces[interfacenr].endpointNumber = endpointNr;
    if ((arraydata.getUint8(pointer+2) & 0x80)> 0)
    {
        result.interfaces[interfacenr].endpointType="IN";
    }
    else
    {
        result.interfaces[interfacenr].endpointType="OUT";
    }
    result.interfaces[interfacenr].endpoints[endpointNr] = {};
    result.interfaces[interfacenr].endpoints[endpointNr].bLength = arraydata.getUint8(pointer);
    result.interfaces[interfacenr].endpoints[endpointNr].bDescriptorType = arraydata.getUint8(pointer+1);
    result.interfaces[interfacenr].endpoints[endpointNr].bmAttributes = arraydata.getUint8(pointer +3);
    var endpointAttributes = arraydata.getUint8(pointer + 3);
    switch (endpointAttributes & 0x3)
    {
        case 0:
            result.interfaces[interfacenr].endpoints[endpointNr].transferType = "Control"
            break;
        case 1:
            result.interfaces[interfacenr].endpoints[endpointNr].transferType = "Isochronous"
            switch ((endpointAttributes >> 2) & 0x3)
            {
                case 0:
                    result.interfaces[interfacenr].endpoints[endpointNr].isochronous.sychronizationType = "NoSynchronization"
                    break;
                case 1:
                    result.interfaces[interfacenr].endpoints[endpointNr].isochronous.sychronizationType = "Asynchronous";
                    break;
                case 2:
                    result.interfaces[interfacenr].endpoints[endpointNr].isochronous.sychronizationType = "Adaptive";
                    break;
                default:
                    result.interfaces[interfacenr].endpoints[endpointNr].isochronous.sychronizationType = "Synchronous";
                    break;
            }
            switch ((endpointAttributes >> 4) & 0x3)
            {
                case 0:
                    result.interfaces[interfacenr].endpoints[endpointNr].isochronous.useageType = "data"
                    break;
                case 1:
                    result.interfaces[interfacenr].endpoints[endpointNr].isochronous.useageType = "feedback";
                    break;
                case 2:
                    result.interfaces[interfacenr].endpoints[endpointNr].isochronous.useageType = "implicitFeedback";
                    break;
                default:
                    result.interfaces[interfacenr].endpoints[endpointNr].isochronous.sychronizationType = "reserved";
                    break;
            }
            break;            
        case 2:
            result.interfaces[interfacenr].endpoints[endpointNr].transferType = "Bulk"
            break; 
        default:
            result.interfaces[interfacenr].endpoints[endpointNr].transferType = "Interrupt"
            break;
    }
    result.interfaces[interfacenr].endpoints[endpointNr].wMaxPacketSize = arraydata.getUint16(pointer+4,true);
    result.interfaces[interfacenr].endpoints[endpointNr].maxPacketSize = result.interfaces[interfacenr].endpoints[endpointNr].maxPacketSize & 0x3FF;
    result.interfaces[interfacenr].enpoints[endpointNr].bInterval = arraydata.getUint8(pointer+6);
}

function parseDfuInterfaceDescriptor(arraydata,pointer,result,interfacenr)
{
    var bmAttributes = arraydata.getUint8(pointer + 2);
    result.interfaces[interfacenr].dfu = {};
    result.interfaces[interfacenr].dfu.bitWillDetach = 0;
    result.interfaces[interfacenr].dfu.bitManifestationTolerant = 0;
    result.interfaces[interfacenr].dfu.bitCanUpload = 0;
    result.interfaces[interfacenr].dfu.bitCanDownload = 0;
    if ((bmAttributes & (1 << 3)) != 0)
    {
        result.interfaces[interfacenr].dfu.bitWillDetach = 1;
    }
    if ((bmAttributes & (1 << 2)) != 0)
    {
        result.interfaces[interfacenr].dfu.bitManifestationTolerant = 1;
    }
    if ((bmAttributes & (1 << 1)) != 0)
    {
        result.interfaces[interfacenr].dfu.bitCanUpload = 1;
    }
    if ((bmAttributes & (1 << 0)) != 0)
    {
        result.interfaces[interfacenr].dfu.bitCanDownload = 1;
    }
    result.interfaces[interfacenr].dfu.wDetachTimeout = arraydata.getUint16(pointer + 3,true);
    result.interfaces[interfacenr].dfu.wTransferSize = arraydata.getUint16(pointer + 5,true);
    var bcdDFUVersionMSB = arraydata.getUint8(pointer + 7);
    var bcdDFUVersionLSB = arraydata.getUint8(pointer + 8);
    result.interfaces[interfacenr].dfu.bcdDFUVersion = "";
    result.interfaces[interfacenr].dfu.bcdDFUVersion += String.fromCharCode(((bcdDFUVersionMSB >> 4) & 0xF) + 0x30);
    result.interfaces[interfacenr].dfu.bcdDFUVersion += String.fromCharCode((bcdDFUVersionMSB & 0xF) + 0x30);   
    result.interfaces[interfacenr].dfu.bcdDFUVersion += "."
    result.interfaces[interfacenr].dfu.bcdDFUVersion += String.fromCharCode(((bcdDFUVersionLSB >> 4) & 0xF) + 0x30);
    result.interfaces[interfacenr].dfu.bcdDFUVersion += String.fromCharCode((bcdDFUVersionLSB & 0xF) + 0x30); 
}

async function firmwareUpgradeStepSTM32()
{
    let dfuStatus;
    let outData=new ArrayBuffer(0);
    switch (firmwareUpgradeState.state)
    {
        case "initial":
            requestDevice();
            break;
        case "deviceDetectedSTM32":
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

            mainConsole.innerText += "\nOpening Device";
            await firmwareUpgradeState.device.open();
            firmwareUpgradeState.stateNr += 1;
            mainConsole.innerText += "\nSelect configuration 0";
            if (firmwareUpgradeState.device.configuration == null)
            {
                await firmwareUpgradeState.device.selectConfiguration(0);
            }
            firmwareUpgradeState.stateNr += 1;
            mainConsole.innerText += "\nClaiming Interface 0";
            await firmwareUpgradeState.device.claimInterface(0);
            firmwareUpgradeState.stateNr += 1;
            mainConsole.innerText += "\nSetting Interface 0";
            
            await firmwareUpgradeState.device.controlTransferOut({requestType: "standard", 
                recipient: "device",
                request: SET_INTERFACE,
                value: 0,
                index: 0},outData);
            firmwareUpgradeState.stateNr += 1;

            dfuStatus = await getDFUStatus(0);
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
                    try {
                        await firmwareUpgradeState.device.releaseInterface(0);

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
                    setTimeout(firmwareUpgradeStepSTM32,dfuStatus.bwPollTimeout);
                } 
                else
                {
                    mainConsole.innerText += '\nDetach Failed';
                }
            }
            else if (dfuStatus.bStatus === 'OK' && dfuStatus.bState === 'APP_DETACH')
            {
                mainConsole.innerText += "\ndevice already detached";
                firmwareUpgradeState.state = "inDfuMode";
                firmwareUpgradeState.stateNr += 1;
            }
            else if (dfuStatus.bStatus === 'OK' && dfuStatus.bState === 'IDLE')
            {
                mainConsole.innerText += "\ndevice already in DFU Mode";
                let configDescriptor = await getConfigurationDescriptor(9);
                configDescriptor = await getConfigurationDescriptor(configDescriptor.wTotalLength);
                let transferSize = 0;
                try {
                    transferSize = configDescriptor.interfaces[0].dfu.wTransferSize;
                }
                catch 
                {
                    mainConsole.innerText += "\ncould not read Dfu functional descriptor";
                    break;
                }

                const fwData =  await fwPromise;
                const totalBytes = fwData.byteLength - 16- 11-274-8; 
                let bytesTransferred=0;
                let blockNum=2;
                let progress=0;
                mainConsole.innerText += "\nStarting Download";

                // erase sector 0
                let dfuseSpecialCmd = new Uint8Array([0x41,0x00,0x00,0x00,0x08]);
                await firmwareUpgradeState.device.controlTransferOut({
                    requestType: "class",
                    recipient: "interface",
                    request: DFU_DNLOAD,
                    value: 0,
                    index: 0
                },dfuseSpecialCmd);
                await waitForDfuStatus('DNLOAD_IDLE');

                // set address
                dfuseSpecialCmd = new Uint8Array([0x21,0x00,0x00,0x00,0x08]);
                await firmwareUpgradeState.device.controlTransferOut({
                    requestType: "class",
                    recipient: "interface",
                    request: DFU_DNLOAD,
                    value: 0,
                    index: 0
                },dfuseSpecialCmd);
                await waitForDfuStatus('DNLOAD_IDLE');

                while (bytesTransferred < totalBytes)
                {
                    let bytesToSend;
                    if (totalBytes - bytesTransferred > transferSize)
                    {
                        bytesToSend = transferSize;
                    }
                    else
                    {
                        bytesToSend = totalBytes - bytesTransferred;
                    }

                    let dchunk = new Uint8Array(firmwareData,(blockNum-2)*transferSize + 11+8+274,bytesToSend); 
                    await firmwareUpgradeState.device.controlTransferOut({requestType: "class", 
                    recipient: "interface",
                    request: DFU_DNLOAD,
                    value: blockNum,
                    index: 0},dchunk);
                    await waitForDfuStatus('DNLOAD_IDLE');
                    bytesTransferred += bytesToSend;
                    if (blockNum == 2)
                    {
                        mainConsole.innerText += "\n";
                    }
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
                await new Promise(r => setTimeout(r,52));
                //wait waitForDfuStatus(["MANIFEST_WAIT_RESET","MANIFEST","MANIFEST_SYNC"]);
                mainConsole.innerText += "\nDownload succeeded";
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
            /*
        case "inDfuMode":
            mainConsole.innerText += "\nget Device again";
            let dev = await navigator.usb.requestDevice({ filters: [{ vendorId: 0x0483, productId: 0xDF11 }] });
            firmwareUpgradeState.device = dev;
            mainConsole.innerText += "\nreopen device (in DFU mode)";
            await firmwareUpgradeState.device.open();
            if (firmwareUpgradeState.device.configuration == null)
            {
                await firmwareUpgradeState.device.selectConfiguration(0);
            }
            if (firmwareUpgradeState.device.configuration.interfaces.length!==1)
            {
                mainConsole.innerText += "\nDevice not in Dfu Mode, stopping";
                break;
            }
            mainConsole.innerText += "\nclaiming interface 0";
            await firmwareUpgradeState.device.claimInterface(0);
            if (dfuFunctionalDescriptor.wTransferSize > 0)
            {
                
            }
            else
            {
                mainConsole.innerText += "\ncould read Dfu functional descriptor";
                break;
            }
            dfuStatus = await getDFUStatus(0);
            mainConsole.innerText += "\n" + JSON.stringify(dfuStatus).replace(/,/g,', ');
            if (dfuStatus.bStatus === 'OK' && dfuStatus.bState === 'IDLE')
            {
                const fwData =  await fwPromise;
                const totalBytes = fwData.byteLength - 16- 11-274-8; 
                let bytesTransferred=0;
                let blockNum=0;
                let progress=0;
                mainConsole.innerText += "\nStarting Download\n";
            
                while (bytesTransferred < totalBytes)
                {
                    let bytesToSend;
                    if (totalBytes - bytesTransferred > dfuFunctionalDescriptor.wTransferSize)
                    {
                        bytesToSend = wTransferSize;
                    }
                    else
                    {
                        bytesToSend = totalBytes - bytesTransferred;
                    }

                    let dchunk = new Uint8Array(firmwareData,blockNum*dfuFunctionalDescriptor.wTransferSize + 11+8+274,bytesToSend); 
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
                        await new Promise(r => setTimeout(r,dlStatus.bwPollTimeout));
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
        */
    }
}

async function waitForDfuStatus(targetState)
{
    let polltimeout = 0;
    let nstalls = 0;
    while(true)
    {
        let dlStatus;
        try {
            dlStatus = await getDFUStatus(0);
        }
        catch (error){
            if ((error === 'stall' || error === 'USBPipeError')  && nstalls < 10)
            {
                nstalls += 1;
                polltimeout = 1000; // set a timeout of 1s and repoll
                mainConsole.innerText += "\nEndpoint stalled, retry " + nstalls;
            }
            else {
                mainConsole.innerText += "\nEndpoint is still stalled, Download failed!";
                throw new Error("waitForDfuStatus failed");
            }
        }
        if (typeof dlStatus !== 'undefined' && dlStatus.bStatus !== 'OK')
        {
            mainConsole.innerText += "\nWrong DFU Status " + dlStatus.bStatus + ", failed!";
            throw new Error("waitForDfuStatus failed");
        }
        
        if (typeof dlStatus !== 'undefined' && dlStatus.bState === targetState)
        {
            polltimeout = dlStatus.bwPollTimeout;
            break;
        }
        let targetStateMatch=false;
        if (typeof dlStatus !== 'undefined')
        {
            if (Array.isArray(targetState))
            {
                for (const el in targetState)
                {
                    targetStateMatch = targetStateMatch || (dlStatus.bState === targetState);
                }
            }
            else
            {
                targetStateMatch = dlStatus.bState === targetState;
            }
        }
        else if (typeof dlStatus !== 'undefined' && !targetStateMatch)
        {
            polltimeout = dlStatus.bwPollTimeout;
        }
        await new Promise(r => setTimeout(r,polltimeout));
    }
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
    .requestDevice({ filters: [{ vendorId: 0x4A37, productId: 0x35D2 },{vendorId: 0x0483, productId: 0xDF11}] })
    .then((usbDevice) => {
        detectDeviceConsole.innerText = "got compatible Device";
        if (usbDevice.vendorId === 0x4A37 && usbDevice.productId === 0x35D2)
        {
            mainConsole.innerText = "\nPiPicoFX detected";
            firmwareUpgradeState.state = "deviceDetectedPPFX";
            populateFirmwaresCombobox((el) => el === "regular");
        }
        else if (usbDevice.vendorId === 0x0483 && usbDevice.productId === 0xDF11)
        {
            mainConsole.innerText = "\nSTM32H750 detected";
            firmwareUpgradeState.state = "deviceDetectedSTM32";
            populateFirmwaresCombobox((el) => el === "firstTime");
        }
        firmwareUpgradeState.stateNr = 1;
        firmwareUpgradeState.device = usbDevice;
        firmwareUpdateButton.disabled=false;
  })
    .catch(() => {
        detectDeviceConsole.innerText = "No Device Present";
  });
}

