import './App.css'
import EffectView from './EffectPrograms';
import ParametersDisplay from './ParametersDisplay';
import Routing from './Routing';
import Console from './Console';
import { useState } from 'react';

let ppfxDevice=null;
let fxProgs;
let commandBfr=[];
let currentCommandLength=0;
let currentCommandNr=0xFFFF;
let commandBfrIdx=0;
let presetBfr=[{},{},{}];
let currentBankBfr=0;
let masterVolumeMessageState = 0;
let parameterValueState = 0;
let parameterValueSending = 0;
let newParameterValues=[];
//let currentPresetBfr=0;
//let readCommandState = 0; // 0: read header, 1: read command
//let readCommandRemainingSize = 0; // size of the command to read, excluding header

const USB_CMD_GET_ABOUT = 0
const USB_CMD_GET_INPUTS_AND_MASTER_VOLUME = 3
const USB_CMD_SET_INPUTS_AND_MASTER_VOLUME = 4
const USB_CMD_GET_CURRENT_BANK_PRESET_NR = 5
const USB_CMD_GET_PRESET = 6
const USB_CMD_SET_PARAMETER = 7
const USB_CMD_GET_PROGRAMS = 8
const USB_CMD_GET_PARAMETER_NAMES = 2
const USB_CMD_LOAD_PRESET=9


const MSG_ABOUT = 0
const MSG_PROGRAMS = 1
const MSG_PARAMETER_NAMES = 2
const MSG_INPUTS_AND_MASTER_VOLUME = 3
const MSG_BANK_AND_PRESET_NR = 5
const MSG_PRESET = 6
const MSG_PARAMETER_VALUE = 7

export async function getParameterNames(programNr)
{
    console.log("getParameterNames(" + programNr + ")");
    let cmdbfr= new ArrayBuffer(5);
    let cmd = new Uint8Array(cmdbfr);
    cmd[0]=USB_CMD_GET_PARAMETER_NAMES;
    cmd[1]=0;
    cmd[2]=5;
    cmd[3]=0;
    cmd[4]=programNr;
    writeIfPossible(cmd);
}

async function writeIfPossible(cmd)
{
    if (ppfxDevice != "undefined")
    {
        if (ppfxDevice.configuration != null)
        {
            await ppfxDevice.transferOut(1,cmd); 
            return 1;
        }
    }
    return 0;
}


function App() {

    const [fxPrograms,setFxPrograms] = useState([
        {
            "id": 0,
            "name": "AmpModel",
            "freezable":false,
            "parameterNames": ["Gain", "Tone","Presence", "Volume"]
        },
        {
            "id": 1,
            "name": "Delay",
            "freezable":true,
            "parameterNames": ["Time", "Feedback", "Mix"]
        },
        {
            "id": 2,
            "name": "Reverb",
            "freezable": true,
            "parameterNames":  ["Time", "Mix"]
        },
        {
            "id": 255,
            "name": "<None>",
            "freezable": false,
            "parameterNames": []
        }
    ]);


    const [presets,setPresets] = useState([
        {
            "bankNr": 1,
            "presetNr": 0,
            "routing": 0,
            "name": "B0 P0",
            "programsAndParameters": [
                {
                    "programNr": 0,
                    "parameters": [50,100,150,200],
                    "displayNames": ["50","100","150","200"],
                    "state": "on"
                },
                {
                    "programNr": 1,
                    "parameters": [400,500,600],
                    "displayNames": ["400","500","600"],
                    "state": "off"
                },
                {
                    "programNr": 255,
                    "parameters": [700,800,900],
                    "displayNames": ["700","800","900"],
                    "state": "on"
                }
            ]
        },
                {
            "bankNr": 1,
            "presetNr": 1,
            "routing": 0,
            "name": "B0 P1",
            "programsAndParameters": [
                {
                    "programNr": 0,
                    "parameters": [200,150,100,50],
                    "displayNames": ["200","150","100","50"],
                    "state": "off"
                },
                {
                    "programNr": 2,
                    "parameters": [400,500,600],
                    "displayNames": ["400","500","600"],
                    "state": "on"
                },
                {
                    "programNr": 255,
                    "parameters": [700,800,900],
                    "displayNames": ["700","800","900"],
                    "state": "on"
                }
            ]
        },
                {
            "bankNr": 1,
            "presetNr": 2,
            "routing": 0,
            "name": "B0 P2",
            "programsAndParameters": [
                {
                    "programNr": 0,
                    "parameters": [200,150,100,50],
                    "displayNames": ["200","150","100","50"],
                    "state": "on"
                },
                {
                    "programNr": 2,
                    "parameters": [400,500,600],
                    "displayNames": ["400","500","600"],
                    "state": "on"
                },
                {
                    "programNr": 1,
                    "parameters": [700,800,900],
                    "displayNames": ["700","800","900"],
                    "state": "off"
                }
            ]
        }
    ]);
    const [consoleText,setConsoleTest] = useState("");
    const [HiZOn,setHiZOn] = useState(false);
    const [MicOn,setMicOn] = useState(true);
    const [masterVolume,setMasterVolume] = useState(200);
    const [currentBank,setCurrentBank] = useState(0);
    const [currentPreset,setCurrentPreset] = useState(0);
    const [currentFxProgramIdx,setCurrentFxProgramIdx]=useState(0);
    const [singleParamValue,setSingleParamValue]=useState(1);
  


    function aboutHandler()
    {
        getAbout();
    }

    function appendToConsole(msg)
    {
        setConsoleTest(consoleText + msg);
    }

    async function getAbout()
    {
        
        let cmdbfr= new ArrayBuffer(4);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_GET_ABOUT;
        cmd[1]=0;
        cmd[2]=4;
        cmd[3]=0;
        await writeIfPossible(cmd); 
    }
    function getProgramsHandler()
    {
        getPrograms();
    }

    async function getPrograms()
    {
        console.log("calling getPrograms()");
        let cmdbfr= new ArrayBuffer(4);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_GET_PROGRAMS;
        cmd[1]=0;
        cmd[2]=4;
        cmd[3]=0;
        await writeIfPossible(cmd);
    }

    async function getInputsAndMasterVolume()
    {
        let cmdbfr= new ArrayBuffer(4);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_GET_INPUTS_AND_MASTER_VOLUME;
        cmd[1]=0;
        cmd[2]=4;
        cmd[3]=0;
        await writeIfPossible(cmd);  
    }

    async function setInputsAndMasterVolume(mVol,hiz,mic)
    {

        let cmdbfr= new ArrayBuffer(6);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_SET_INPUTS_AND_MASTER_VOLUME;
        cmd[1]=0;
        cmd[2]=6;
        cmd[3]=0;
        cmd[4]=0;
        if (hiz === true)
        {
            cmd[4] |= 1;
        }
        if (mic === true)
        {
            cmd[4] |= 2;
        }
        cmd[5]=mVol;
        await writeIfPossible(cmd);
        await readCommand();
        masterVolumeMessageState=0;
    }

    async function loadCurrentPreset(presetIdx)
    {
        let cmdbfr= new ArrayBuffer(5);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_LOAD_PRESET;
        cmd[1]=0;
        cmd[2]=5;
        cmd[3]=0;
        cmd[4]=presetIdx;
        await writeIfPossible(cmd);
    }

    async function setParameter()
    {
        const dataset = newParameterValues.pop();
        if (dataset != undefined)
        {
            let cmdbfr= new ArrayBuffer(8);
            let cmd = new Uint8Array(cmdbfr);
            cmd[0]=USB_CMD_SET_PARAMETER;
            cmd[1]=0;
            cmd[2]=8;
            cmd[3]=0;
            cmd[4]=dataset.programIdx;
            cmd[5]=dataset.parameterIdx;
            cmd[6]=dataset.parameterVal & 0xFF;
            cmd[7]=(dataset.parameterVal >> 8) &0xFF;
            await writeIfPossible(cmd);  
            console.log("sending programIdx: " + dataset.programIdx + ", parameterIdx: " + dataset.parameterIdx + ", parameterVal: " + dataset.parameterVal);
            await readCommand();
            parameterValueSending = 0;
        }   
    }

    async function getCurrentBankAndPresetNr()
    {
        let cmdbfr= new ArrayBuffer(4);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_GET_CURRENT_BANK_PRESET_NR;
        cmd[1]=0;
        cmd[2]=4;
        cmd[3]=0;
        await writeIfPossible(cmd);
    }

    async function getPreset(bankNr,presetNr)
    {
        let cmdbfr= new ArrayBuffer(6);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_GET_PRESET;
        cmd[1]=0;
        cmd[2]=6;
        cmd[3]=0;
        cmd[4]=bankNr;
        cmd[5]=presetNr;
        await writeIfPossible(cmd); 
    } 



    async function readCommand()
    {
        let transferResult;
        let isResolved = false;
        while (!isResolved)
        {
            if (currentCommandNr !== 0xFFFF)
            {
                let alignedSize = (Math.floor(currentCommandLength/64) + 1)*64;
                transferResult = await ppfxDevice.transferIn(1,alignedSize); 
            }
            else
            {
                transferResult = await ppfxDevice.transferIn(1,64); 
            }
            if (transferResult.status !== "ok")
            {
                let data = new Uint8Array(transferResult.data.buffer);
                console.log("readCommand(), returned unexpected status " + transferResult.status + " when reading message header");
                parameterValueState = 0;
                masterVolumeMessageState = 0;
                currentCommandNr=0xFFFF;
            }
            else
            {
                commandBfr = new Uint8Array(transferResult.data.buffer);
                if (commandBfr.length == 4)
                {
                    currentCommandNr = transferResult.data.getUint16(0,true);
                    currentCommandLength = transferResult.data.getUint16(2,true);
                }
                if (currentCommandNr !== 0xFFFF && commandBfr.length === currentCommandLength)
                {
                    switch(currentCommandNr)
                    {
                        case MSG_ABOUT:
                            commandBfrIdx = processAboutMessage(commandBfr,0,currentCommandLength)
                            console.log("handled MSG_ABOUT");
                            break;
                        case MSG_PROGRAMS:
                            commandBfrIdx = processGetPrograms(commandBfr,0,currentCommandLength);
                            console.log("handled MSG_PROGRAMS");
                            break;
                        case MSG_INPUTS_AND_MASTER_VOLUME:
                            commandBfrIdx = processInputAndMasterVolume(commandBfr,0,currentCommandLength);
                            console.log("handled MSG_INPUTS_AND_MASTER_VOLUME");
                            masterVolumeMessageState = 0;
                            break;
                        case MSG_PARAMETER_NAMES:
                            commandBfrIdx = processGetParameterNames(commandBfr,0,currentCommandLength);
                            console.log("handled MSG_PARAMETER_NAMES");
                            break;
                        case MSG_BANK_AND_PRESET_NR:
                            commandBfrIdx = processCurrentBankAndPresetNr(commandBfr,0,currentCommandLength);
                            console.log("handled MSG_BANK_AND_PRESET_NR");
                            break;
                        case MSG_PRESET:
                            commandBfrIdx = processGetPreset(commandBfr,0,currentCommandLength);
                            console.log("handled MSG_PRESET");
                            break;
                        case MSG_PARAMETER_VALUE:
                            commandBfrIdx = processParameterDisplayValue(commandBfr,0,currentCommandLength);
                            console.log("handled MSG_PARAMETER_VALUE");
                            parameterValueState = 0;
                            break;
                        default:
                            console.log("unknown command " + commandBfr[0] + " " + commandBfr[1]);
                            parameterValueState = 0;
                            break;
                    }
                    currentCommandNr = 0xFFFF;
                    currentCommandLength = 0xFFFF;
                    isResolved=true;
                    
                    //}
                    //else
                    //{
                    //    console.log("Message length error,read " + commandBfr.length + " bytes, expected " + currentCommandLength);
                    //    parameterValueState = 0;
                    //    masterVolumeMessageState = 0;
                    //}
                }
            }
        }
    }

    async function readFromPort()
    {
        while (true)
        {
            await readCommand();
        }
    }

    async function initialSync()
    {
        await getPrograms();
        await readCommand();
        for (const fxp of fxProgs)
        {
            await getParameterNames(fxp.id);
            await readCommand();
        }
        await getInputsAndMasterVolume();
        await readCommand();
        await getCurrentBankAndPresetNr();
        await readCommand();
        await getPreset(currentBankBfr,0);
        await readCommand();
        await getPreset(currentBankBfr,1);
        await readCommand();
        await getPreset(currentBankBfr,2);
        await readCommand();
        setFxPrograms(fxProgs);
        setPresets(presetBfr);

    }

    function setParameterValue(position,value)
    {
        const updatedPresets = presets.slice();
        updatedPresets[currentPreset].programsAndParameters[currentFxProgramIdx].parameters[position]=value;
        setPresets(updatedPresets);
        if (newParameterValues.length < 2)
        {
            newParameterValues.push({programIdx: currentFxProgramIdx,parameterIdx:position, parameterVal: value/1});
        }
        else
        {
            newParameterValues[1] = {programIdx: currentFxProgramIdx,parameterIdx:position, parameterVal: value/1};
        }
        if (parameterValueSending === 0)
        {
            parameterValueSending = 1;
            setParameter();
        }        
    }

    function updateRouting(routingId)
    {
        const updatedPresets = presets.slice();
        updatedPresets[currentPreset].routing = routingId;
        setPresets(updatedPresets);
    }

    function setEffectState(position,state)
    {
        const updatedPresets = presets.slice();
        if (state == 1)
        {
            updatedPresets[currentPreset].programsAndParameters[position].state = "on";
        }
        else
        {
            updatedPresets[currentPreset].programsAndParameters[position].state = "off";
        }
        setPresets(updatedPresets);
    }

    function processAboutMessage(msg,idx,size)
    {
        appendToConsole(String.fromCharCode(...msg.slice(idx,idx + size)));
        return idx+size;
    }

    
    function processGetPrograms(msg,msgIdx,size)
    {
        let n_programs = msg[msgIdx + 4];
        let progCnt=0;
        fxProgs=new Array(n_programs);
        let idx=msgIdx + 5;
        while (idx < msgIdx + size)
        {
            
            let fxProg={ParameterCount: msg[idx]& 0x7F,freezable: false,name: "",id: progCnt, parameterNames: []};
            if ((msg[idx] & 0x80)>0)
            {
                fxProg.freezable=true;
            }
            let nameArray=[];
            idx++;
            while(msg[idx]!=0 && idx < msgIdx + size)
            {
                nameArray.push(msg[idx++]);
            }
            fxProg.name=String.fromCharCode(...nameArray);
            fxProgs[progCnt++] = fxProg;
            idx++;
        }
        return msgIdx + size;
    }

    function processInputAndMasterVolume(msg,msgIdx,size)
    {
        setHiZOn(((msg[msgIdx + 4] & 1)>> 0) == 1);
        setMicOn(((msg[msgIdx + 4] & 2) >> 1) == 1);
        setMasterVolume(msg[msgIdx + 5]);
        return msgIdx + size;
    }


    function processGetParameterNames(msg,msgIdx,size)
    {

        let parameterNamesLocal=[];
        let nameArray=[];
        let idx=msgIdx + 5;
        while (idx < msgIdx + size)
        {
            nameArray=[];
            while(msg[idx]!=0 && idx < msg.length)
            {
                nameArray.push(msg[idx++]);
            }
            idx++;
            parameterNamesLocal.push(String.fromCharCode(...nameArray));
        }
        fxProgs[msg[msgIdx + 4]].parameterNames = parameterNamesLocal;
        return msgIdx + size;
    }

    function processCurrentBankAndPresetNr(msg,msgIdx,size)
    {
        currentBankBfr = msg[msgIdx + 4];
        //currentPresetBfr = msg[msgIdx + 5];
        setCurrentBank(msg[msgIdx + 4]);
        setCurrentPreset(msg[msgIdx + 5]);
        return msgIdx + size;
    }

    function processGetPreset(msg,msgIdx,size)
    {
        let nameArray=[];
        let idx=msgIdx + 7;
        let cnt=0;
        let preset={};
        preset["bankNr"]=msg[msgIdx + 4];
        preset["presetNr"] = msg[msgIdx + 5] & 0x3;
        preset["routing"] = (msg[msgIdx + 5] >> 2);
        const effectAState = (msg[msgIdx + 6] & 3);
        const effectBState = ((msg[msgIdx + 6] >>2) & 3);
        const effectCState = ((msg[msgIdx + 6] >>4) & 3);        
        nameArray = [];
        while(msg[idx]!=0 && idx < msgIdx + size)
        {
            nameArray.push(msg[idx++]);
        }
        idx++;
        preset["name"]=String.fromCharCode(...nameArray);
        preset["programsAndParameters"]=[];
        preset["programsAndParameters"].push({"programNr":msg[idx++],"parameters": [],"displayNames": [],  "state": "off"});
        if (effectAState == 0)
        {
            preset["programsAndParameters"][0]["state"] = "off";
        }
        else if (effectAState == 1)
        {
            preset["programsAndParameters"][0]["state"] = "on";
        }
        else if (effectAState == 2)
        {
            preset["programsAndParameters"][0]["state"] = "frozen";
        }
        if (preset["programsAndParameters"][0]["programNr"] !== 0xFF)
        {
            cnt=0;
            while (cnt < fxProgs[preset["programsAndParameters"][0]["programNr"]].parameterNames.length)
            {
                preset["programsAndParameters"][0]["parameters"].push(msg[idx] | (msg[idx+1]<<8));
                idx+=2;
                nameArray=[];
                while(msg[idx]!=0 && idx < msg.length)
                {
                    nameArray.push(msg[idx++]);
                }
                idx++;
                preset["programsAndParameters"][0]["displayNames"].push(String.fromCharCode(...nameArray)); 
                cnt++;
            }
        }

        preset["programsAndParameters"].push({"programNr":msg[idx++],"parameters": [],"displayNames": [], "state": "off"});
        if (effectBState == 0)
        {
            preset["programsAndParameters"][1]["state"] = "off";
        }
        else if (effectBState == 1)
        {
            preset["programsAndParameters"][1]["state"] = "on";
        }
        else if (effectBState == 2)
        {
            preset["programsAndParameters"][1]["state"] = "frozen";
        }
        if (preset["programsAndParameters"][1]["programNr"] !== 0xFF)
        {
            cnt=0;
            while (cnt < fxProgs[preset["programsAndParameters"][1]["programNr"]].parameterNames.length)
            {
                preset["programsAndParameters"][1]["parameters"].push(msg[idx] | (msg[idx+1]<<8));
                idx+=2;
                nameArray=[];
                while(msg[idx]!=0 && idx < msg.length)
                {
                    nameArray.push(msg[idx++]);
                }
                idx++;
                preset["programsAndParameters"][1]["displayNames"].push(String.fromCharCode(...nameArray)); 
                cnt++;
            }
        }

        preset["programsAndParameters"].push({"programNr":msg[idx++],"parameters": [],"displayNames": [], "state": "off"});
        if (effectCState == 0)
        {
            preset["programsAndParameters"][2]["state"] = "off";
        }
        else if (effectCState == 1)
        {
            preset["programsAndParameters"][2]["state"] = "on";
        }
        else if (effectCState == 2)
        {
            preset["programsAndParameters"][2]["state"] = "frozen";
        }
        if (preset["programsAndParameters"][2]["programNr"] !== 0xFF)
        {
            cnt=0;
            while (cnt < fxProgs[preset["programsAndParameters"][2]["programNr"]].parameterNames.length)
            {
                preset["programsAndParameters"][2]["parameters"].push(msg[idx] | (msg[idx+1]<<8));
                nameArray=[];
                while(msg[idx]!=0 && idx < msg.length)
                {
                    nameArray.push(msg[idx++]);
                }
                idx++;
                preset["programsAndParameters"][2]["displayNames"].push(String.fromCharCode(...nameArray)); 
                idx+=2;
                cnt++;
            }
        }
        presetBfr[preset["presetNr"]]=preset;
        return msgIdx + size;
    }

    function processParameterDisplayValue(msg,msgIdx,size)
    {
        let idx = msgIdx + 6;
        let nameArray=[];
        while(msg[idx]!=0 && idx < msgIdx + size)
        {
            nameArray.push(msg[idx++]);
        }
        idx++;
        const effectIndex = msg[msgIdx + 4];
        const parameterIdx = msg[msgIdx + 5];
        const updatedPresets = presetBfr.slice();
        updatedPresets[currentPreset].programsAndParameters[effectIndex].displayNames[parameterIdx] = String.fromCharCode(...nameArray);
        setPresets(updatedPresets);
        return msgIdx + size;
    }

    function requestDevice()
    {
        let devFound=false;
        navigator.usb.getDevices().then(async (devices) => {
            devices.forEach( async d => {
                if (d.vendorId == 0x4A37 && d.productId == 0x35D2)
                {
                    ppfxDevice = d;
                    devFound = true;
                    await ppfxDevice.open();
                    if (ppfxDevice.configuration == null)
                    {
                        await ppfxDevice.selectConfiguration(0);
                    }
                    await ppfxDevice.claimInterface(1);
                    await initialSync();
                    //readFromPort();
                }
            });
            if (devFound === false)
            {
                navigator.usb.requestDevice({ filters: [{ vendorId: 0x4A37, productId: 0x35D2 }] }).then( async (usbDevice) =>
                {
                    
                    ppfxDevice = usbDevice;
                    await ppfxDevice.open();
                    if (ppfxDevice.configuration == null)
                    {
                        await ppfxDevice.selectConfiguration(0);
                    }
                    await ppfxDevice.claimInterface(1);
                    await initialSync();
                    //readFromPort();
                    appendToConsole("PiPicoFX USB Device Opened");

                }).catch((error) => {
                    appendToConsole("Error: " + error + ", No Device Found or selected");
                });
            }
        });
    }



    function previousBankHandler()
    {

    }

    function nextBankHandler()
    {

    }

    function handleMasterVolumeChange(value)
    {
        
        if (masterVolumeMessageState===0)
        {
            masterVolumeMessageState=1;
            console.log("setting current volume " + value);
            setInputsAndMasterVolume(value,HiZOn,MicOn);
            //setMasterVolume(value);
            
        }
    }

    function handleHiZChange(value)
    {
        if (masterVolumeMessageState===0)
        {
            masterVolumeMessageState=1;
            console.log("setting current input state for HiZ " + value);
            setInputsAndMasterVolume(masterVolume,value,MicOn);
            //setHiZOn(value);
        }
    }

    function handleMicChange(value)
    {
        if (masterVolumeMessageState===0)
        {
            masterVolumeMessageState=1;
            console.log("setting current input state for Mic " + MicOn);
            setInputsAndMasterVolume(masterVolume,HiZOn,value);
            //setMicOn(value);
        }
    }
  return (
    <>
<div className="editor-main" id="editor_app">
	<div className="editor-title">Editor</div>
	<div className="editor-toppanel">
		<div className="editor-vertical">
			<input type="range" id="volume" name="volume" min="0" max="255" value={masterVolume} 
                    onChange={(e) => handleMasterVolumeChange(e.target.value)}/>
			<div className="editor-vertical-label">Master Volume</div>
		</div>
		<div className="editor-vertical">
				<label className="editor-switch">
					<input type="checkbox" checked={HiZOn} 
                    onChange={(e) => handleHiZChange(e.target.checked)}/>
					<span className="editor-slider round"></span>
				</label>

				<div className="editor-toggle-label">HiZ</div>
		</div>
		<div className="editor-vertical">
				<label className="editor-switch">
					<input type="checkbox" checked={MicOn} 
                    onChange={(e) => handleMicChange(e.target.checked)}/>
					<span className="editor-slider round"></span>
				</label>

				<div className="editor-toggle-label">Mic</div>
		</div>
		<button className="editor-button" onClick={requestDevice}>Detect device</button>
	</div>
	<div className="editor-panel">
		<div className="editor-vertical">
			<div id="currentPreset" className="editor-textfield">{presets[currentPreset].name}</div>
			<div  className="editor-vertical-label">Preset</div>
		</div>
		<div className="editor-vertical">
			<div id="currentPosition" className="editor-textfield">{currentPreset}</div>
			<div  className="editor-vertical-label">Position</div>
		</div>
		<div className="editor-vertical">
			<div id="currentBank" className="editor-textfield">{currentBank}</div>
			<div  className="editor-vertical-label">Bank</div>
		</div>
		<span className="editor-filler"></span>
	</div>
	<div className="editor-panel">
		<button className="editor-button" onClick={previousBankHandler}>Previous Bank</button>
		<button className="editor-button" onClick={nextBankHandler}>Next Bank</button>
		
		<div className="editor-vertical">
			<input type="radio" name="presetNr" value="a" id="presetNr1" className="editor-text-selectable" checked={currentPreset==0} onChange={() => {
                setCurrentPreset(0);
                loadCurrentPreset(0);
                }}></input>
            <label htmlFor="presetNr1" id="presetNr1Label">{presets[0].name}</label>
			<input type="radio" name="presetNr" value="b" id="presetNr2"  className="editor-text-selectable" checked={currentPreset==1} onChange={() => {
                setCurrentPreset(1);
                loadCurrentPreset(1);
            }}></input>
            <label htmlFor="presetNr2" id="presetNr2Label">{presets[1].name}</label>
			<input type="radio" name="presetNr" value="c" id="presetNr3"  className="editor-text-selectable" checked={currentPreset==2} onChange={() => {
                setCurrentPreset(2);
                loadCurrentPreset(2);
                }}></input>
            <label htmlFor="presetNr3" id="presetNr3Label">{presets[2].name}</label>
		</div>
		<span className="editor-filler"></span>
        <EffectView 
            id="0" 
            fxPrograms={fxPrograms} 
            presets={presets} 
            currentPreset={currentPreset} 
            changePresets = {setPresets}
            changeEffectState={setEffectState}
            setCurrentFxProgramIdx={setCurrentFxProgramIdx}
            />
        <EffectView 
            id="1" 
            fxPrograms={fxPrograms} 
            presets={presets} 
            currentPreset={currentPreset}  
            changePresets = {setPresets}
            changeEffectState={setEffectState}
            setCurrentFxProgramIdx={setCurrentFxProgramIdx}
            />
        <EffectView 
            id="2" 
            fxPrograms={fxPrograms} 
            presets={presets} 
            changePresets = {setPresets}
            changeEffectState={setEffectState}
            currentPreset={currentPreset} 
            setCurrentFxProgramIdx={setCurrentFxProgramIdx}
            />
        <Routing presets={presets} currentPreset={currentPreset} changeRouting={updateRouting}/>
	</div>
    <ParametersDisplay preset={presets[currentPreset]} fxPrograms={fxPrograms} currentPreset={currentPreset} effectIndex={currentFxProgramIdx} changeParameterValue={setParameterValue}/>
	<span className="editor-filler-vertical"></span>
    <Console content={consoleText} />
	<div className="editor-panel">
    	<button className="editor-button" onClick={aboutHandler}>Get About</button>
		<button className="editor-button" onClick={getProgramsHandler}>Get Programs</button>
        <button className="editor-button" onClick={getInputsAndMasterVolume}>Get Inputs and Volume</button>
        <button className="editor-button" onClick={() => getParameterNames(1)}>Param Names of first prog</button>
        <button className="editor-button" onClick={() => 
            {setParameterValue(0,singleParamValue);
                setSingleParamValue(() => singleParamValue+1);
            }}>Set First Parameter</button>
		<span className="editor-filler"></span> 
	</div>
</div> 
    </>
  )
}

export default App
