import './App.css'
import EffectView from './EffectPrograms';
import ParametersDisplay from './ParametersDisplay';
import Console from './Console';
import { useState } from 'react';

let ppfxDevice=null;
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


function App() {

    const [fxPrograms,setFxPrograms] = useState([
        {
            "id": 1,
            "name": "AmpModel",
            "freezable":false
        },
        {
            "id": 2,
            "name": "Delay",
            "freezable":true
        }
    ]);
    const [fxParameters,setFxParameters] = useState([
        {
            "id": 1,
            "name": "volume",
            "displayName": "Gain",
            "value": "5754"
        },
        {
            "id": 2,
            "name": "posteq",
            "displayName": "Post-EQ",
            "value": "5623"
        },
        {
            "id": 3,
            "name": "presetvol",
            "displayName": "Preset Vol",
            "value": "70.1%"
        }
    ]);
    const [consoleText,setConsoleTest] = useState("");
    const [HiZOn,setHiZOn] = useState(false);
    const [MicOn,setMicOn] = useState(true);
    const [masterVolume,setMasterVolume] = useState(200);

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

    function processAboutMessage(msg)
    {
        appendToConsole(String.fromCharCode(...msg.slice(4,msg.length)));
    }

    function processGetPrograms(msg)
    {
        let n_programs = msg[4];
        let progCnt=0;
        let fxProgs=new Array(n_programs);
        let idx=5;
        while (idx < msg.length)
        {
            
            let fxProg={ParameterCount: msg[idx]& 0x7F,freezable: false,name: "",id: progCnt};
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
        setFxPrograms(fxProgs);
    }

    function processInputAndMasterVolume(msg)
    {
        setHiZOn((msg[4] & 1) === 1);
        setMicOn(((msg[4] & 2) >> 1) === 1);
        setMasterVolume(msg[5]);
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
                    if (currentCommandLength === 0 && currentCommandNr === 0xFFFF)
                    {
                        currentCommandNr = (value[0] | (value[1] << 8));
                        currentCommandLength = (value[2] | (value[3] << 8));
                        commandBfr = commandBfr.concat([].slice.call(value));
                        console.log("msg received");
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
                            case MSG_INPUTS_AND_MASTER_VOLUME:
                                processInputAndMasterVolume(commandBfr);
                                console.log("handled MSG_INPUTS_AND_MASTER_VOLUME");
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
                        appendToConsole("PiPicoFX VCom Port Opened");
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
                        appendToConsole("PiPicoFX VCom Port Opened");
                        ppfxDevice = p;
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
			<input type="range" id="volume" name="volume" min="0" max="255" defaultValue={masterVolume} 
                    onChange={(e) => handleMasterVolumeChange(e.target.value)}/>
			<div className="editor-vertical-label">Master Volume</div>
		</div>
		<div className="editor-vertical">
				<label className="editor-switch">
					<input type="checkbox" defaultChecked={HiZOn} 
                    onChange={(e) => handleHiZChange(e.target.checked)}/>
					<span className="editor-slider round"></span>
				</label>

				<div className="editor-toggle-label">HiZ</div>
		</div>
		<div className="editor-vertical">
				<label className="editor-switch">
					<input type="checkbox" defaultChecked={MicOn} 
                    onChange={(e) => handleMicChange(e.target.checked)}/>
					<span className="editor-slider round"></span>
				</label>

				<div className="editor-toggle-label">Mic</div>
		</div>
		<button className="editor-button" onClick={requestDevice}>Detect device</button>
	</div>
	<div className="editor-panel">
		<div className="editor-vertical">
			<div id="currentPreset" className="editor-textfield">DirtyChorus</div>
			<div  className="editor-vertical-label">Preset</div>
		</div>
		<div className="editor-vertical">
			<div id="currentPosition" className="editor-textfield">2</div>
			<div  className="editor-vertical-label">Position</div>
		</div>
		<div className="editor-vertical">
			<div id="currentBank" className="editor-textfield">1</div>
			<div  className="editor-vertical-label">Bank</div>
		</div>
		<span className="editor-filler"></span>
	</div>
	<div className="editor-panel">
		<button className="editor-button" onClick={previousBankHandler}>Previous Bank</button>
		<button className="editor-button" onClick={nextBankHandler}>Next Bank</button>
		
		<div className="editor-vertical">
			<input type="radio" name="presetNr" value="a" id="presetNr1" className="editor-text-selectable" defaultChecked></input><label htmlFor="presetNr1" id="presetNr1Label">DirtyChorus</label>
			<input type="radio" name="presetNr" value="b" id="presetNr2"  className="editor-text-selectable" ></input><label htmlFor="presetNr2" id="presetNr2Label">B1 P2</label>
			<input type="radio" name="presetNr" value="c" id="presetNr3"  className="editor-text-selectable" ></input><label htmlFor="presetNr3" id="presetNr3Label">B1 P3</label>
		</div>
		<span className="editor-filler"></span>
        <EffectView id="A" fxPrograms={fxPrograms}/>
        <EffectView id="B" fxPrograms={fxPrograms}/>
        <EffectView id="C" fxPrograms={fxPrograms}/>
	</div>
    <ParametersDisplay parameters={fxParameters} />
	<span className="editor-filler-vertical"></span>
    <Console content={consoleText} />
	<div className="editor-panel">
    	<button className="editor-button" onClick={aboutHandler}>Get About</button>
		<button className="editor-button" onClick={getProgramsHandler}>Get Programs</button>
        <button className="editor-button" onClick={getInputsAndMasterVolume}>Get Inputs and Volume</button>
		<span className="editor-filler"></span> 
	</div>
</div> 
    </>
  )
}

export default App
