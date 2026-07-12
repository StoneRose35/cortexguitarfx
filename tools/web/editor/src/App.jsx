import './App.css'
import EffectView from './EffectPrograms';
import ParametersDisplay from './ParametersDisplay';
import Routing from './Routing';
import Console from './Console';
import { useState } from 'react';
import {
    USB_CMD_GET_ABOUT,
    USB_CMD_GET_INPUTS_AND_MASTER_VOLUME,
    USB_CMD_SET_INPUTS_AND_MASTER_VOLUME,
    USB_CMD_GET_CURRENT_BANK_PRESET_NR,
    USB_CMD_GET_PRESET,
    USB_CMD_SET_PARAMETER,
    USB_CMD_GET_PROGRAMS,
    USB_CMD_GET_PARAMETER_NAMES,
    USB_CMD_LOAD_PRESET,
    USB_CMD_SET_FX_PROGRAM,
    USB_CMD_FXPROGRAM_ON_OFF,
    MSG_ABOUT,
    MSG_PROGRAMS,
    MSG_PARAMETER_NAMES,
    MSG_INPUTS_AND_MASTER_VOLUME,
    MSG_BANK_AND_PRESET_NR,
    MSG_PRESET,
    MSG_PARAMETER_VALUE
} from './editorConstants';
import { createInitialFxPrograms, createInitialPresets } from './editorData';
import {
    processAboutMessage,
    processGetPrograms,
    processInputAndMasterVolume,
    processGetParameterNames,
    processCurrentBankAndPresetNr,
    processParameterDisplayValue
} from './editorMessageHandlers';

let ppfxDevice = null;
let fxProgs;
let commandBfr = [];
let currentCommandLength = 0;
let currentCommandNr = 0xFFFF;
let commandBfrIdx = 0;
let presetBfr = [{}, {}, {}];
let currentBankBfr = 0;
let masterVolumeMessageState = 0;
let parameterValueState = 0;
let parameterValueSending = 0;
let newParameterValues = [];
//let currentPresetBfr=0;
//let readCommandState = 0; // 0: read header, 1: read command
//let readCommandRemainingSize = 0; // size of the command to read, excluding header

function App() {

    const [fxPrograms, setFxPrograms] = useState(createInitialFxPrograms());
    const [presets, setPresets] = useState(createInitialPresets());
    const [consoleText, setConsoleTest] = useState('');
    const [HiZOn, setHiZOn] = useState(false);
    const [MicOn, setMicOn] = useState(true);
    const [masterVolume, setMasterVolume] = useState(200);
    const [currentBank, setCurrentBank] = useState(0);
    const [currentPreset, setCurrentPreset] = useState(0);
    const [currentFxProgramIdx, setCurrentFxProgramIdx] = useState(0);
    const [singleParamValue, setSingleParamValue] = useState(1);
    const [preset0Name,setPreset0Name] = useState("");
    const [preset1Name,setPreset1Name] = useState("");
    const [preset2Name,setPreset2Name] = useState("");
    const [presetSelected,setPresetSelected] = useState(true);

    function aboutHandler() {
        getAbout();
    }

    function appendToConsole(msg) {
        setConsoleTest((previousText) => previousText + msg);
    }

    async function getAbout() {
        const cmdbfr = new ArrayBuffer(4);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_GET_ABOUT;
        cmd[1] = 0;
        cmd[2] = 4;
        cmd[3] = 0;
        await writeIfPossible(cmd);
    }

    function getProgramsHandler() {
        getPrograms();
    }

    async function getPrograms() {
        console.log('calling getPrograms()');
        const cmdbfr = new ArrayBuffer(4);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_GET_PROGRAMS;
        cmd[1] = 0;
        cmd[2] = 4;
        cmd[3] = 0;
        await writeIfPossible(cmd);
    }

    async function getInputsAndMasterVolume() {
        const cmdbfr = new ArrayBuffer(4);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_GET_INPUTS_AND_MASTER_VOLUME;
        cmd[1] = 0;
        cmd[2] = 4;
        cmd[3] = 0;
        await writeIfPossible(cmd);
    }

    async function setInputsAndMasterVolume(mVol, hiz, mic) {
        const cmdbfr = new ArrayBuffer(6);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_SET_INPUTS_AND_MASTER_VOLUME;
        cmd[1] = 0;
        cmd[2] = 6;
        cmd[3] = 0;
        cmd[4] = 0;
        if (hiz === true) {
            cmd[4] |= 1;
        }
        if (mic === true) {
            cmd[4] |= 2;
        }
        cmd[5] = mVol;
        await writeIfPossible(cmd);
        await readCommand();
        masterVolumeMessageState = 0;
    }

    async function loadPreset(presetIdx) {
        const cmdbfr = new ArrayBuffer(6);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_LOAD_PRESET;
        cmd[1] = 0;
        cmd[2] = 6;
        cmd[3] = 0;
        cmd[4] = currentBank;
        cmd[5] = presetIdx;
        await writeIfPossible(cmd);
    }

    async function setParameter() {
        let dataset = newParameterValues.pop();
        while (dataset !== undefined) {
            const cmdbfr = new ArrayBuffer(8);
            const cmd = new Uint8Array(cmdbfr);
            cmd[0] = USB_CMD_SET_PARAMETER;
            cmd[1] = 0;
            cmd[2] = 8;
            cmd[3] = 0;
            cmd[4] = dataset.programIdx;
            cmd[5] = dataset.parameterIdx;
            cmd[6] = dataset.parameterVal & 0xFF;
            cmd[7] = (dataset.parameterVal >> 8) & 0xFF;
            await writeIfPossible(cmd);
            console.log(`sending programIdx: ${dataset.programIdx}, parameterIdx: ${dataset.parameterIdx}, parameterVal: ${dataset.parameterVal}`);
            await readCommand();
            dataset = newParameterValues.pop();
        }
        parameterValueSending = 0;
    }

    async function getCurrentBankAndPresetNr() {
        const cmdbfr = new ArrayBuffer(4);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_GET_CURRENT_BANK_PRESET_NR;
        cmd[1] = 0;
        cmd[2] = 4;
        cmd[3] = 0;
        await writeIfPossible(cmd);
    }

    async function getPreset(bankNr, presetNr) {
        const cmdbfr = new ArrayBuffer(6);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_GET_PRESET;
        cmd[1] = 0;
        cmd[2] = 6;
        cmd[3] = 0;
        cmd[4] = bankNr;
        cmd[5] = presetNr;
        await writeIfPossible(cmd);
    }

    async function setFxProgram(position, programNr) {
        const cmdbfr = new ArrayBuffer(6);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_SET_FX_PROGRAM;
        cmd[1] = 0;
        cmd[2] = 6;
        cmd[3] = 0;
        cmd[4] = position;
        cmd[5] = programNr;
        await writeIfPossible(cmd);
    }

    async function switchFxProgramOnOff(position, value) {
        const cmdbfr = new ArrayBuffer(6);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_FXPROGRAM_ON_OFF;
        cmd[1] = 0;
        cmd[2] = 6;
        cmd[3] = 0;
        cmd[4] = position;
        cmd[5] = value;
        await writeIfPossible(cmd);
    }

    function processGetPreset(msg, msgIdx, size, fxProgs) {
        let idx = msgIdx + 7;
        const preset = {};
        preset.bankNr = msg[msgIdx + 4];
        preset.presetNr = msg[msgIdx + 5] & 0x3;
        preset.routing = msg[msgIdx + 5] >> 2;

        const effectAState = msg[msgIdx + 6] & 3;
        const effectBState = (msg[msgIdx + 6] >> 2) & 3;
        const effectCState = (msg[msgIdx + 6] >> 4) & 3;

        let nameArray = [];
        while (msg[idx] !== 0 && idx < msgIdx + size) {
            nameArray.push(msg[idx++]);
        }
        idx += 1;
        preset.name = String.fromCharCode(...nameArray);
        preset.programsAndParameters = [];

        preset.programsAndParameters.push({
            programNr: msg[idx++],
            parameters: [],
            displayNames: [],
            state: 'off'
        });
        if (effectAState === 0) {
            preset.programsAndParameters[0].state = 'off';
        } else if (effectAState === 1) {
            preset.programsAndParameters[0].state = 'on';
        } else if (effectAState === 2) {
            preset.programsAndParameters[0].state = 'frozen';
        }

        if (preset.programsAndParameters[0].programNr < 63) {
            let cnt = 0;
            while (cnt < fxProgs[preset.programsAndParameters[0].programNr].parameterNames.length) {
                preset.programsAndParameters[0].parameters.push(msg[idx] | (msg[idx + 1] << 8));
                idx += 2;
                nameArray = [];
                while (msg[idx] !== 0 && idx < msg.length) {
                    nameArray.push(msg[idx++]);
                }
                idx += 1;
                preset.programsAndParameters[0].displayNames.push(String.fromCharCode(...nameArray));
                cnt += 1;
            }
        }

        preset.programsAndParameters.push({
            programNr: msg[idx++],
            parameters: [],
            displayNames: [],
            state: 'off'
        });
        if (effectBState === 0) {
            preset.programsAndParameters[1].state = 'off';
        } else if (effectBState === 1) {
            preset.programsAndParameters[1].state = 'on';
        } else if (effectBState === 2) {
            preset.programsAndParameters[1].state = 'frozen';
        }

        if (preset.programsAndParameters[1].programNr < 63) {
            let cnt = 0;
            while (cnt < fxProgs[preset.programsAndParameters[1].programNr].parameterNames.length) {
                preset.programsAndParameters[1].parameters.push(msg[idx] | (msg[idx + 1] << 8));
                idx += 2;
                nameArray = [];
                while (msg[idx] !== 0 && idx < msg.length) {
                    nameArray.push(msg[idx++]);
                }
                idx += 1;
                preset.programsAndParameters[1].displayNames.push(String.fromCharCode(...nameArray));
                cnt += 1;
            }
        }

        preset.programsAndParameters.push({
            programNr: msg[idx++],
            parameters: [],
            displayNames: [],
            state: 'off'
        });
        if (effectCState === 0) {
            preset.programsAndParameters[2].state = 'off';
        } else if (effectCState === 1) {
            preset.programsAndParameters[2].state = 'on';
        } else if (effectCState === 2) {
            preset.programsAndParameters[2].state = 'frozen';
        }

        if (preset.programsAndParameters[2].programNr < 63) {
            let cnt = 0;
            while (cnt < fxProgs[preset.programsAndParameters[2].programNr].parameterNames.length) {
                preset.programsAndParameters[2].parameters.push(msg[idx] | (msg[idx + 1] << 8));
                idx += 2;
                nameArray = [];
                while (msg[idx] !== 0 && idx < msg.length) {
                    nameArray.push(msg[idx++]);
                }
                idx += 1;
                preset.programsAndParameters[2].displayNames.push(String.fromCharCode(...nameArray));
                cnt += 1;
            }
        }
        if (preset.presetNr === 0)
        {
            setPreset0Name(preset.name);
        }
        else if (preset.presetNr === 1)
        {
            setPreset1Name(preset.name);
        }
        else
        {
            setPreset2Name(preset.name);
        }

        return { preset, nextIdx: msgIdx + size };
    }
    async function readCommand() {
        let transferResult;
        let isResolved = false;
        while (!isResolved) {
            if (currentCommandNr !== 0xFFFF) {
                const alignedSize = (Math.floor(currentCommandLength / 64) + 1) * 64;
                transferResult = await ppfxDevice.transferIn(1, alignedSize);
            } else {
                transferResult = await ppfxDevice.transferIn(1, 64);
            }
            if (transferResult.status !== 'ok') {
                const data = new Uint8Array(transferResult.data.buffer);
                console.log(`readCommand(), returned unexpected status ${transferResult.status} when reading message header`);
                parameterValueState = 0;
                masterVolumeMessageState = 0;
                currentCommandNr = 0xFFFF;
            } else {
                commandBfr = new Uint8Array(transferResult.data.buffer);
                if (commandBfr.length === 4) {
                    currentCommandNr = transferResult.data.getUint16(0, true);
                    currentCommandLength = transferResult.data.getUint16(2, true);
                }
                if (currentCommandNr !== 0xFFFF && commandBfr.length === currentCommandLength) {
                    switch (currentCommandNr) {
                        case MSG_ABOUT:
                            commandBfrIdx = processAboutMessage(commandBfr, 0, currentCommandLength, appendToConsole);
                            console.log('handled MSG_ABOUT');
                            break;
                        case MSG_PROGRAMS: {
                            const programResult = processGetPrograms(commandBfr, 0, currentCommandLength);
                            fxProgs = programResult.fxProgs;
                            commandBfrIdx = programResult.nextIdx;
                            console.log('handled MSG_PROGRAMS');
                            break;
                        }
                        case MSG_INPUTS_AND_MASTER_VOLUME: {
                            const volumeResult = processInputAndMasterVolume(commandBfr, 0, currentCommandLength);
                            setHiZOn(volumeResult.hiZOn);
                            setMicOn(volumeResult.micOn);
                            setMasterVolume(volumeResult.masterVolume);
                            commandBfrIdx = volumeResult.nextIdx;
                            console.log('handled MSG_INPUTS_AND_MASTER_VOLUME');
                            masterVolumeMessageState = 0;
                            break;
                        }
                        case MSG_PARAMETER_NAMES: {
                            const parameterResult = processGetParameterNames(commandBfr, 0, currentCommandLength, fxProgs);
                            fxProgs = parameterResult.fxProgs;
                            commandBfrIdx = parameterResult.nextIdx;
                            console.log('handled MSG_PARAMETER_NAMES');
                            break;
                        }
                        case MSG_BANK_AND_PRESET_NR: {
                            const bankResult = processCurrentBankAndPresetNr(commandBfr, 0, currentCommandLength);
                            currentBankBfr = bankResult.bank;
                            setCurrentBank(bankResult.bank);
                            setCurrentPreset(bankResult.preset);
                            commandBfrIdx = bankResult.nextIdx;
                            console.log('handled MSG_BANK_AND_PRESET_NR');
                            break;
                        }
                        case MSG_PRESET: {
                            const presetResult = processGetPreset(commandBfr, 0, currentCommandLength, fxProgs);
                            presetBfr[presetResult.preset.presetNr] = presetResult.preset;
                            commandBfrIdx = presetResult.nextIdx;
                            console.log('handled MSG_PRESET');
                            break;
                        }
                        case MSG_PARAMETER_VALUE: {
                            const displayValueResult = processParameterDisplayValue(commandBfr, 0, currentCommandLength, presetBfr, currentPreset);
                            presetBfr = displayValueResult.presetBfr;
                            setPresets(presetBfr);
                            commandBfrIdx = displayValueResult.nextIdx;
                            console.log('handled MSG_PARAMETER_VALUE');
                            parameterValueState = 0;
                            break;
                        }
                        default:
                            console.log(`unknown command ${commandBfr[0]} ${commandBfr[1]}`);
                            parameterValueState = 0;
                            break;
                    }
                    currentCommandNr = 0xFFFF;
                    currentCommandLength = 0xFFFF;
                    isResolved = true;
                }
            }
        }
    }

    async function readFromPort() {
        while (true) {
            await readCommand();
        }
    }

    async function initialSync() {
        await getPrograms();
        await readCommand();
        for (const fxp of fxProgs) {
            if (fxp.id < 63)
            {
                await getParameterNames(fxp.id);
                await readCommand();
            }
        }
        await getInputsAndMasterVolume();
        await readCommand();
        await getCurrentBankAndPresetNr();
        await readCommand();
        await getPreset(currentBankBfr, 0);
        await readCommand();
        await getPreset(currentBankBfr, 1);
        await readCommand();
        await getPreset(currentBankBfr, 2);
        await readCommand();
        setFxPrograms(fxProgs);
        setPresets(presetBfr);
    }

    function setParameterValue(position, value) {
        const updatedPresets = presets.slice();
        updatedPresets[currentPreset].programsAndParameters[currentFxProgramIdx].parameters[position] = value;
        setPresets(updatedPresets);
        if (newParameterValues.length < 2) {
            newParameterValues.push({ programIdx: currentFxProgramIdx, parameterIdx: position, parameterVal: value / 1 });
        } else {
            newParameterValues[1] = { programIdx: currentFxProgramIdx, parameterIdx: position, parameterVal: value / 1 };
        }
        if (parameterValueSending === 0) {
            parameterValueSending = 1;
            setParameter();
        }
    }

    function updateRouting(routingId) {
        const updatedPresets = presets.slice();
        updatedPresets[currentPreset].routing = routingId;
        setPresets(updatedPresets);
    }

    function setEffectState(position, state) {
        if (presets[currentPreset].programsAndParameters[position].programNr < 63)
        {
            const updatedPresets = presets.slice();
            if (state === true) {
                updatedPresets[currentPreset].programsAndParameters[position].state = 'on';
            } else {
                updatedPresets[currentPreset].programsAndParameters[position].state = 'off';
            }
            setPresets(updatedPresets);
            switchFxProgramOnOff(position, state);
        }
    }

    async function writeIfPossible(cmd) {
        if (ppfxDevice !== null) {
            if (ppfxDevice.configuration !== null) {
                await ppfxDevice.transferOut(1, cmd);
                return 1;
            }
        }
        return 0;
    }

    async function getParameterNames(programNr) {
        console.log(`getParameterNames(${programNr})`);
        const cmdbfr = new ArrayBuffer(5);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_GET_PARAMETER_NAMES;
        cmd[1] = 0;
        cmd[2] = 5;
        cmd[3] = 0;
        cmd[4] = programNr;
        writeIfPossible(cmd);
    }

    function requestDevice() {
        let devFound = false;
        navigator.usb.getDevices().then(async (devices) => {
            devices.forEach(async (d) => {
                if (d.vendorId === 0x4A37 && d.productId === 0x35D2) {
                    ppfxDevice = d;
                    devFound = true;
                    await ppfxDevice.open();
                    if (ppfxDevice.configuration === null) {
                        await ppfxDevice.selectConfiguration(0);
                    }
                    await ppfxDevice.claimInterface(1);
                    await initialSync();
                    //readFromPort();
                }
            });
            if (devFound === false) {
                navigator.usb.requestDevice({ filters: [{ vendorId: 0x4A37, productId: 0x35D2 }] }).then(async (usbDevice) => {
                    ppfxDevice = usbDevice;
                    await ppfxDevice.open();
                    if (ppfxDevice.configuration === null) {
                        await ppfxDevice.selectConfiguration(0);
                    }
                    await ppfxDevice.claimInterface(1);
                    await initialSync();
                    //readFromPort();
                    appendToConsole('PiPicoFX USB Device Opened');
                }).catch((error) => {
                    appendToConsole(`Error: ${error}, No Device Found or selected`);
                });
            }
        });
    }

    async function previousBankHandler() {
        if (currentBankBfr > 0)
        {
            currentBankBfr -= 1;
            //setCurrentPreset(-1);
            await getPreset(currentBankBfr, 0);
            await readCommand();
            await getPreset(currentBankBfr, 1);
            await readCommand();
            await getPreset(currentBankBfr, 2);
            await readCommand();
            setPresets(presetBfr);
            setCurrentBank(currentBankBfr);
            setPresetSelected(false);
        }
    }

    async function nextBankHandler() {
            currentBankBfr += 1;
            //setCurrentPreset(-1);
            await getPreset(currentBankBfr, 0);
            await readCommand();
            await getPreset(currentBankBfr, 1);
            await readCommand();
            await getPreset(currentBankBfr, 2);
            await readCommand();
            setPresets(presetBfr);
            setCurrentBank(currentBankBfr);
            setPresetSelected(false);
    }

    function handleMasterVolumeChange(value) {
        if (masterVolumeMessageState === 0) {
            masterVolumeMessageState = 1;
            console.log(`setting current volume ${value}`);
            setInputsAndMasterVolume(value, HiZOn, MicOn);
            //setMasterVolume(value);
        }
    }

    function handleHiZChange(value) {
        if (masterVolumeMessageState === 0) {
            masterVolumeMessageState = 1;
            console.log(`setting current input state for HiZ ${value}`);
            setInputsAndMasterVolume(masterVolume, value, MicOn);
            //setHiZOn(value);
        }
    }

    function handleMicChange(value) {
        if (masterVolumeMessageState === 0) {
            masterVolumeMessageState = 1;
            console.log(`setting current input state for Mic ${MicOn}`);
            setInputsAndMasterVolume(masterVolume, HiZOn, value);
            //setMicOn(value);
        }
    }

    async function handleFxProgramChange(id, fxProgram, preset) {
        setPresets(preset);
        await setFxProgram(id, fxProgram);
        await getPreset(currentBank, currentPreset);
        await readCommand();
        setPresets(presetBfr);
        //setCurrentFxProgramIdx(fxProgram/1);
    }

    return (
        <>
            <div className="editor-main" id="editor_app">
                <div className="editor-title">Editor</div>
                <div className="editor-toppanel">
                    <div className="editor-vertical">
                        <input type="range" id="volume" name="volume" min="0" max="255" value={masterVolume}
                            onChange={(e) => handleMasterVolumeChange(e.target.value)} />
                        <div className="editor-vertical-label">Master Volume</div>
                    </div>
                    <div className="editor-vertical">
                        <label className="editor-switch">
                            <input type="checkbox" checked={HiZOn}
                                onChange={(e) => handleHiZChange(e.target.checked)} />
                            <span className="editor-slider round"></span>
                        </label>

                        <div className="editor-toggle-label">HiZ</div>
                    </div>
                    <div className="editor-vertical">
                        <label className="editor-switch">
                            <input type="checkbox" checked={MicOn}
                                onChange={(e) => handleMicChange(e.target.checked)} />
                            <span className="editor-slider round"></span>
                        </label>

                        <div className="editor-toggle-label">Mic</div>
                    </div>
                    <button className="editor-button" onClick={requestDevice}>Detect device</button>
                </div>
                <div className="editor-panel">
                    <div className="editor-vertical">
                        <div id="currentPreset" className="editor-textfield">{presets[currentPreset].name}</div>
                        <div className="editor-vertical-label">Preset</div>
                    </div>
                    <div className="editor-vertical">
                        <div id="currentPosition" className="editor-textfield">{currentPreset}</div>
                        <div className="editor-vertical-label">Position</div>
                    </div>
                    <div className="editor-vertical">
                        <div id="currentBank" className="editor-textfield">{currentBank}</div>
                        <div className="editor-vertical-label">Bank</div>
                    </div>
                    <span className="editor-filler"></span>
                </div>
                <div className="editor-panel">
                    <button className="editor-button" onClick={previousBankHandler}>Previous Bank</button>
                    <button className="editor-button" onClick={nextBankHandler}>Next Bank</button>

                    <div className="editor-vertical-flex">
                        <input type="radio" name="presetNr" value="a" id="presetNr1" className="editor-text-selectable" checked={currentPreset === 0 && presetSelected} onChange={() => {
                            setPresetSelected(true);
                            setCurrentPreset(0);
                            loadPreset(0);
                        }}></input>
                        <label htmlFor="presetNr1" id="presetNr1Label">{preset0Name}</label>
                        <input type="radio" name="presetNr" value="b" id="presetNr2" className="editor-text-selectable" checked={currentPreset === 1 && presetSelected} onChange={() => {
                            setPresetSelected(true);
                            setCurrentPreset(1);
                            loadPreset(1);
                        }}></input>
                        <label htmlFor="presetNr2" id="presetNr2Label">{preset1Name}</label>
                        <input type="radio" name="presetNr" value="c" id="presetNr3" className="editor-text-selectable" checked={currentPreset === 2 && presetSelected} onChange={() => {
                            setPresetSelected(true);
                            setCurrentPreset(2);
                            loadPreset(2);
                        }}></input>
                        <label htmlFor="presetNr3" id="presetNr3Label">{preset2Name}</label>
                    </div>
                    <span className="editor-filler"></span>
                    <EffectView
                        id="0"
                        fxPrograms={fxPrograms}
                        presets={presets}
                        currentPreset={currentPreset}
                        changePresets={handleFxProgramChange}
                        changeEffectState={setEffectState}
                        setCurrentFxProgramIdx={setCurrentFxProgramIdx}
                    />
                    <EffectView
                        id="1"
                        fxPrograms={fxPrograms}
                        presets={presets}
                        currentPreset={currentPreset}
                        changePresets={handleFxProgramChange}
                        changeEffectState={setEffectState}
                        setCurrentFxProgramIdx={setCurrentFxProgramIdx}
                    />
                    <EffectView
                        id="2"
                        fxPrograms={fxPrograms}
                        presets={presets}
                        changePresets={handleFxProgramChange}
                        changeEffectState={setEffectState}
                        currentPreset={currentPreset}
                        setCurrentFxProgramIdx={setCurrentFxProgramIdx}
                    />
                    <Routing presets={presets} currentPreset={currentPreset} changeRouting={updateRouting} />
                </div>
                <ParametersDisplay preset={presets[currentPreset]} fxPrograms={fxPrograms} effectIndex={currentFxProgramIdx} changeParameterValue={setParameterValue} />
                <span className="editor-filler-vertical"></span>
                <Console content={consoleText} />
                <div className="editor-panel">
                    <button className="editor-button" onClick={aboutHandler}>Get About</button>
                    <button className="editor-button" onClick={getProgramsHandler}>Get Programs</button>
                    <button className="editor-button" onClick={getInputsAndMasterVolume}>Get Inputs and Volume</button>
                    <button className="editor-button" onClick={() => getParameterNames(1)}>Param Names of first prog</button>
                    <button className="editor-button" onClick={() => {
                        setParameterValue(0, singleParamValue);
                        setSingleParamValue(() => singleParamValue + 1);
                    }}>Set First Parameter</button>
                    <span className="editor-filler"></span>
                </div>
            </div>
        </>
    );
}

export default App;
