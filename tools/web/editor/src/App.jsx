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
let presetBfr=[{},{},{}];
let currentBankBfr=0;
let currentPresetBfr=0;

const USB_CMD_GET_ABOUT = 0
const USB_CMD_GET_INPUTS_AND_MASTER_VOLUME = 3
const USB_CMD_SET_INPUTS_AND_MASTER_VOLUME = 4
const USB_CMD_GET_CURRENT_BANK_PRESET_NR = 5
const USB_CMD_GET_PRESET = 6
const USB_CMD_GET_PROGRAMS = 8
const USB_CMD_GET_PARAMETER_NAMES = 2


const MSG_ABOUT = 0
const MSG_PROGRAMS = 1
const MSG_PARAMETER_NAMES = 2
const MSG_INPUTS_AND_MASTER_VOLUME = 3
const MSG_BANK_AND_PRESET_NR = 5
const MSG_PRESET = 6

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
    const writer = ppfxDevice.writable.getWriter();
    await writer.write(cmd);
    writer.releaseLock();  
}


    export 

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

    const [fxParameters,setFxParameters] = useState([
        {
            "id": 0,
            "displayName": "Gain",
            "displayValue": "5754",
            "rawValue": 2345
        },
        {
            "id": 1,
            "displayName": "Post-EQ",
            "displayValue": "5623",
            "rawValue": 2645
        },
        {
            "id": 2,
            "displayName": "Preset Vol",
            "displayValue": "70.1%",
            "rawValue": 1345
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
                    "state": "on"
                },
                {
                    "programNr": 1,
                    "parameters": [400,500,600],
                    "state": "off"
                },
                {
                    "programNr": 255,
                    "parameters": [700,800,900],
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
                    "state": "off"
                },
                {
                    "programNr": 2,
                    "parameters": [400,500,600],
                    "state": "on"
                },
                {
                    "programNr": 255,
                    "parameters": [700,800,900],
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
                    "state": "on"
                },
                {
                    "programNr": 2,
                    "parameters": [400,500,600],
                    "state": "on"
                },
                {
                    "programNr": 1,
                    "parameters": [700,800,900],
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

    async function getInputsAndMasterVolume()
    {
        let cmdbfr= new ArrayBuffer(4);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_GET_INPUTS_AND_MASTER_VOLUME;
        cmd[1]=0;
        cmd[2]=4;
        cmd[3]=0;
        const writer = ppfxDevice.writable.getWriter();
        await writer.write(cmd);
        writer.releaseLock();  
    }

    async function setInputsAndMasterVolume()
    {
        let cmdbfr= new ArrayBuffer(6);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_SET_INPUTS_AND_MASTER_VOLUME;
        cmd[1]=0;
        cmd[2]=6;
        cmd[3]=0;
        cmd[4]=0;
        if (HiZOn === true)
        {
            cmd[4] |= 1;
        }
        if (MicOn === true)
        {
            cmd[4] |= 2;
        }
        cmd[5]=masterVolume;
        const writer = ppfxDevice.writable.getWriter();
        await writer.write(cmd);
        writer.releaseLock();  
    }

    async function getCurrentBankAndPresetNr()
    {
        let cmdbfr= new ArrayBuffer(4);
        let cmd = new Uint8Array(cmdbfr);
        cmd[0]=USB_CMD_GET_CURRENT_BANK_PRESET_NR;
        cmd[1]=0;
        cmd[2]=4;
        cmd[3]=0;
        const writer = ppfxDevice.writable.getWriter();
        await writer.write(cmd);
        writer.releaseLock();  
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
        const writer = ppfxDevice.writable.getWriter();
        await writer.write(cmd);
        writer.releaseLock();  
    } 

    async function readCommand()
    {
        const reader = ppfxDevice.readable.getReader();
        try {
            let imDone=false;
            while(imDone === false)
            {
                const { value, done } = await reader.read();
                if (done)
                {
                    break;
                }
                if (currentCommandLength === 0 && currentCommandNr === 0xFFFF)
                {
                    currentCommandNr = (value[0] | (value[1] << 8));
                    currentCommandLength = (value[2] | (value[3] << 8));
                    commandBfr = commandBfr.concat([].slice.call(value));
                    console.log("msg received, currentCommandLength: " + currentCommandLength + ", commandBfr.length: " + commandBfr.length );
                }
                else if (currentCommandNr !== 0xFFFF || commandBfr.length < 4)
                {
                    commandBfr = commandBfr.concat([].slice.call(value));
                    if (commandBfr.length >= 4)
                    {
                        currentCommandNr = commandBfr[0] | (commandBfr[1] << 8);
                        currentCommandLength = commandBfr[2] | (commandBfr[3] << 8);
                    }
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
                        case MSG_INPUTS_AND_MASTER_VOLUME:
                            processInputAndMasterVolume(commandBfr);
                            console.log("handled MSG_INPUTS_AND_MASTER_VOLUME");
                            break;
                        case MSG_PARAMETER_NAMES:
                            processGetParameterNames(commandBfr);
                            console.log("handled MSG_PARAMETER_NAMES");
                            break;
                        case MSG_BANK_AND_PRESET_NR:
                            processCurrentBankAndPresetNr(commandBfr);
                            console.log("handled MSG_BANK_AND_PRESET_NR");
                            break;
                        case MSG_PRESET:
                            processGetPreset(commandBfr);
                            console.log("handled MSG_PRESET");
                            break;
                        default:
                            console.log("unknown command " + commandBfr[0] + " " + commandBfr[1]);
                            break;
                    }
                    
                    commandBfr=[];
                    currentCommandLength=0;
                    currentCommandNr=0xFFFF;
                    imDone=true;
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
        await  getCurrentBankAndPresetNr();
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
        const updatedFxParams = fxParameters.slice();
        updatedFxParams[position].rawValue = value;
        setFxParameters(updatedFxParams);
        setPresets(updatedPresets);
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

    function processAboutMessage(msg)
    {
        appendToConsole(String.fromCharCode(...msg.slice(4,msg.length)));
    }

    async function processGetPrograms(msg)
    {
        let n_programs = msg[4];
        let progCnt=0;
        fxProgs=new Array(n_programs);
        let idx=5;
        while (idx < msg.length)
        {
            
            let fxProg={ParameterCount: msg[idx]& 0x7F,freezable: false,name: "",id: progCnt, parameterNames: []};
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
            fxProgs[progCnt++] = fxProg;
            idx++;
        }
    }

    function processInputAndMasterVolume(msg)
    {
        setHiZOn((msg[4] & 1) === 1);
        setMicOn(((msg[4] & 2) >> 1) === 1);
        setMasterVolume(msg[5]);
    }


    function processGetParameterNames(msg)
    {

        let parameterNamesLocal=[];
        let nameArray=[];
        let idx=5;
        while (idx < msg.length)
        {
            nameArray=[];
            while(msg[idx]!=0 && idx < msg.length)
            {
                nameArray.push(msg[idx++]);
            }
            idx++;
            parameterNamesLocal.push(String.fromCharCode(...nameArray));
        }
        fxProgs[msg[4]].parameterNames = parameterNamesLocal;
    }

    function processCurrentBankAndPresetNr(msg)
    {
        currentBankBfr = msg[4];
        currentPresetBfr = msg[5];
        setCurrentBank(msg[4]);
        setCurrentPreset(msg[5]);
    }

    function processGetPreset(msg)
    {
        let nameArray=[];
        let idx=6;
        let cnt=0;
        let preset={};
        preset["bankNr"]=msg[4];
        preset["presetNr"] = msg[5] & 0x3;
        preset["routing"] = (msg[5] >> 2);
        nameArray = [];
        while(msg[idx]!=0 && idx < msg.length)
        {
            nameArray.push(msg[idx++]);
        }
        idx++;
        preset["name"]=String.fromCharCode(...nameArray);
        preset["programsAndParameters"]=[];
        preset["programsAndParameters"]=push({"programNr":msg[idx++],"parameters": []});
        if (preset["programsAndParameters"][0]["programNr"] !== 0xFF)
        {
            cnt=0;
            while (cnt < fxProgs[preset["programsAndParameters"][0]["programNr"]].parameterNames.length)
            {
               preset["programsAndParameters"][0]["parameters"].push(msg[idx] | (msg[idx+1]<<8));
                idx+=2;
                cnt++;
            }
        }

        preset["programsAndParameters"]=push({"programNr":msg[idx++],"parameters": []});
        if (preset["programsAndParameters"][1]["programNr"] !== 0xFF)
        {
            cnt=0;
            while (cnt < fxProgs[preset["programsAndParameters"][1]["programNr"]].parameterNames.length)
            {
               preset["programsAndParameters"][1]["parameters"].push(msg[idx] | (msg[idx+1]<<8));
                idx+=2;
                cnt++;
            }
        }

        preset["programsAndParameters"]=push({"programNr":msg[idx++],"parameters": []});
        if (preset["programsAndParameters"][2]["programNr"] !== 0xFF)
        {
            cnt=0;
            while (cnt < fxProgs[preset["programsAndParameters"][2]["programNr"]].parameterNames.length)
            {
               preset["programsAndParameters"][2]["parameters"].push(msg[idx] | (msg[idx+1]<<8));
                idx+=2;
                cnt++;
            }
        }
        presetBfr[preset["presetNr"]]=preset;
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
                    p.open({baudRate: 115200}).then(async () => {
                        appendToConsole("PiPicoFX VCom Port Opened");
                        ppfxDevice = p;
                        await initialSync();
                        readFromPort();
                    });

                }
            });
            if (devFound === false)
            {
                navigator.serial.requestPort({ filters: [{ usbVendorId: 0x4A37,usbProductId: 0x35D2 }] }).then((p) =>
                {
                    p.open({baudRate: 115200}).then(async () => {
                        appendToConsole("PiPicoFX VCom Port Opened");
                        ppfxDevice = p;
                        await initialSync();
                        readFromPort();
                    }).catch(() =>{
                        appendToConsole("Failed to Open VCom Port");
                    });
                }).catch(() => {
                    appendToConsole("No Device Found or selected");
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
        setMasterVolume(value);
        console.log("setting current volume " + masterVolume);
        setInputsAndMasterVolume();
    }

    function handleHiZChange(value)
    {
        setHiZOn(value);
        console.log("setting current input state for HiZ " + value);
        setInputsAndMasterVolume();
    }

    function handleMicChange(value)
    {
        setMicOn(value);
        console.log("setting current input state for Mic " + MicOn);
        setInputsAndMasterVolume();
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
			<input type="radio" name="presetNr" value="a" id="presetNr1" className="editor-text-selectable" defaultChecked onChange={() => setCurrentPreset(0)}></input>
            <label htmlFor="presetNr1" id="presetNr1Label">{presets[0].name}</label>
			<input type="radio" name="presetNr" value="b" id="presetNr2"  className="editor-text-selectable" onChange={() => setCurrentPreset(1)}></input>
            <label htmlFor="presetNr2" id="presetNr2Label">{presets[1].name}</label>
			<input type="radio" name="presetNr" value="c" id="presetNr3"  className="editor-text-selectable" onChange={() => setCurrentPreset(2)}></input>
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
            changeFxParams={setFxParameters}/>
        <EffectView 
            id="1" 
            fxPrograms={fxPrograms} 
            presets={presets} 
            currentPreset={currentPreset}  
            changePresets = {setPresets}
            changeEffectState={setEffectState}
            setCurrentFxProgramIdx={setCurrentFxProgramIdx}
            changeFxParams={setFxParameters}/>
        <EffectView 
            id="2" 
            fxPrograms={fxPrograms} 
            presets={presets} 
            changePresets = {setPresets}
            changeEffectState={setEffectState}
            currentPreset={currentPreset} 
            setCurrentFxProgramIdx={setCurrentFxProgramIdx}
            changeFxParams={setFxParameters}/>
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
		<span className="editor-filler"></span> 
	</div>
</div> 
    </>
  )
}

export default App
