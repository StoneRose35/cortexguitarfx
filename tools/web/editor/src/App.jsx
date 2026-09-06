import './App.css'
import EffectView from './EffectPrograms';
import ParametersDisplay from './ParametersDisplay';
import Routing from './Routing';
import SaveDialog from './SaveDialog';
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
    USB_CMD_SET_PRESET_NAME,
    MSG_ABOUT,
    MSG_PROGRAMS,
    MSG_PARAMETER_NAMES,
    MSG_INPUTS_AND_MASTER_VOLUME,
    MSG_BANK_AND_PRESET_NR,
    MSG_PRESET,
    MSG_PARAMETER_VALUE,
    USB_CMD_SET_ROUTING,
    USB_CMD_SAVE_PRESET,
    USB_CMD_SET_LED_COLOR
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
import ParameterSlider from './ParameterSlider';
import VolumeSlider from './VolumeSlider';

let ppfxDevice = null;
let fxProgs;
let commandBfr = [];
let currentCommandLength = 0;
let currentCommandNr = 0xFFFF;
let commandBfrIdx = 0;
let presetBfr = [{}, {}, {}];
let savePresetBfr = [{}, {}, {}];
let currentBankBfr = 0;
let masterVolumeMessageState = 0;
let parameterValueSending = 0;
let presetNameChanging = 0;
let newParameterValue = null;
//let currentPresetBfr=0;
//let readCommandState = 0; // 0: read header, 1: read command
//let readCommandRemainingSize = 0; // size of the command to read, excluding header

async function writeIfPossible(cmd) {
    if (ppfxDevice !== null) {
        if (ppfxDevice.configuration !== null) {
            await ppfxDevice.transferOut(1, cmd);
            return 1;
        }
    }
    return 0;
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

export async function getBankContent(bankNr) {

    await getPreset(bankNr, 0);
    await readPreset();
    await getPreset(bankNr, 1);
    await readPreset();
    await getPreset(bankNr, 2);
    await readPreset();
    return savePresetBfr;
}

   export async function savePreset(preset)
    {
        let scnt=0;
        const bfr1 = new ArrayBuffer(512);
        const cbfr1 = new Uint8Array(bfr1);
        cbfr1[scnt++] = USB_CMD_SAVE_PRESET;
        cbfr1[scnt++] = 0;
        scnt+=2;
        cbfr1[scnt++] = preset.bankNr;
        cbfr1[scnt++] = preset.presetNr | (preset.routing << 2);
        let allstates = 0;
        
        if (preset.programsAndParameters[0].state == 'on')
        {
            allstates |= 1 << 0;
        }
        else if (preset.programsAndParameters[0].state == 'frozen')
        {
            allstates |= 2 << 0;
        }
        if (preset.programsAndParameters[1].state == 'on')
        {
            allstates |= 1 << 2;
        }
        else if (preset.programsAndParameters[1].state == 'frozen')
        {
            allstates |= 2 << 2;
        }
        if (preset.programsAndParameters[2].state == 'on')
        {
            allstates |= 1 << 4;
        }
        else if (preset.programsAndParameters[2].state == 'frozen')
        {
            allstates |= 2 << 4;
        }

        cbfr1[scnt++] = allstates; 
        cbfr1[scnt++] = preset.ledColorA | (preset.ledColorB << 2) | (preset.ledColorC << 4) | (preset.ledColorPreset << 6);
        const encoder = new TextEncoder();
        const presetNameUint8 = encoder.encode(preset.name);
        for (let c=0;c< presetNameUint8.byteLength;c++)
        {
            cbfr1[scnt++] = presetNameUint8[c];
        }
        cbfr1[scnt++] = 0;
        cbfr1[scnt++] = preset.programsAndParameters[0].programNr;
        if (preset.programsAndParameters[0].programNr < 63)
        {
            let cnt = 0;
            while (cnt < fxProgs[preset.programsAndParameters[0].programNr].parameterNames.length) {
                cbfr1[scnt++] = preset.programsAndParameters[0].parameters[cnt] & 0xFF;
                cbfr1[scnt++] = (preset.programsAndParameters[0].parameters[cnt] >> 8) & 0xFF;
                //const displayVal =  encoder.encode(preset.programsAndParameters[0].displayNames[cnt]);
                //for (let c=0;c<displayVal.byteLength;c++)
                //{
                //    cbfr1[scnt++] = displayVal[c];
                //}    
                //cbfr1[scnt++] = 0;
                cnt++;
            }
        }

        cbfr1[scnt++] = preset.programsAndParameters[1].programNr;
        if (preset.programsAndParameters[1].programNr < 63)
        {
            let cnt = 0;
            while (cnt < fxProgs[preset.programsAndParameters[1].programNr].parameterNames.length) {
                cbfr1[scnt++] = preset.programsAndParameters[1].parameters[cnt] & 0xFF;
                cbfr1[scnt++] = (preset.programsAndParameters[1].parameters[cnt] >> 8) & 0xFF;
                //const displayVal =  encoder.encode(preset.programsAndParameters[1].displayNames[cnt]);
                //for (let c=0;c<displayVal.byteLength;c++)
                //{
                //    cbfr1[scnt++] = displayVal[c];
                //}    
                //cbfr1[scnt++] = 0;
                cnt++;
            }
        }

        cbfr1[scnt++] = preset.programsAndParameters[2].programNr;
        if (preset.programsAndParameters[2].programNr < 63)
        {
            let cnt = 0;
            while (cnt < fxProgs[preset.programsAndParameters[2].programNr].parameterNames.length) {
                cbfr1[scnt++] = preset.programsAndParameters[2].parameters[cnt] & 0xFF;
                cbfr1[scnt++] = (preset.programsAndParameters[2].parameters[cnt] >> 8) & 0xFF;
                //const displayVal =  encoder.encode(preset.programsAndParameters[2].displayNames[cnt]);
                //for (let c=0;c<displayVal.byteLength;c++)
                //{
                //    cbfr1[scnt++] = displayVal[c];
                //}    
                //cbfr1[scnt++] = 0;
                cnt++;
            }
        }
        const cmdbfr = new ArrayBuffer(scnt);
        const cmd = new Uint8Array(cmdbfr);
        for (let c=0;c<scnt;c++)
        {
            cmd[c] = cbfr1[c];
        }
        cmd[2] = scnt & 0xFF;
        cmd[3] = (scnt >> 8) & 0xFF;
        await writeIfPossible(cmd);
    }

async function readPreset() {
        let transferResult;
        let isResolved = false;
        while (!isResolved) {
            if (currentCommandNr !== 0xFFFF) {
                const alignedSize = (Math.floor((currentCommandLength-1) / 64) + 1) * 64;
                transferResult = await ppfxDevice.transferIn(1, alignedSize);
            } else {
                transferResult = await ppfxDevice.transferIn(1, 64);
            }
            if (transferResult.status !== 'ok') {
                const data = new Uint8Array(transferResult.data.buffer);
                console.log(`readCommand(), returned unexpected status ${transferResult.status} when reading message header`);
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
                        case MSG_PRESET: {
                            const presetResult = processGetPreset(commandBfr, 0, currentCommandLength, fxProgs);
                            savePresetBfr[presetResult.preset.presetNr] = presetResult.preset;
                            commandBfrIdx = presetResult.nextIdx;
                            console.log('handled MSG_PRESET in readPreset');
                            break;
                        }
                        default:
                            console.log(`unknown or unexpected command ${commandBfr[0]} ${commandBfr[1]} in readPreset`);
                            break;
                    }
                    currentCommandNr = 0xFFFF;
                    currentCommandLength = 0xFFFF;
                    isResolved = true;
                }
            }
        }
    }


    function processGetPreset(msg, msgIdx, size, fxProgs) {
        let idx = msgIdx + 8;
        const preset = {};
        preset.bankNr = msg[msgIdx + 4];
        preset.presetNr = msg[msgIdx + 5] & 0x3;
        preset.routing = msg[msgIdx + 5] >> 2;
        preset.ledColorA = msg[msgIdx + 6] & 0x3;
        preset.ledColorB = (msg[msgIdx + 6] >> 2) & 0x3;
        preset.ledColorC = (msg[msgIdx + 6] >> 4) & 0x3;
        preset.ledColorPreset = (msg[msgIdx + 6] >> 6) & 0x3;
        const effectAState = msg[msgIdx + 7] & 3;
        const effectBState = (msg[msgIdx + 7] >> 2) & 3;
        const effectCState = (msg[msgIdx + 7] >> 4) & 3;

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

        return { preset, nextIdx: msgIdx + size };
    }

function App() {

    const [fxPrograms, setFxPrograms] = useState(createInitialFxPrograms());
    const [presets, setPresets] = useState(createInitialPresets());
    const [HiZOn, setHiZOn] = useState(false);
    const [MicOn, setMicOn] = useState(true);
    const [masterVolume, setMasterVolume] = useState(200);
    const [currentBank, setCurrentBank] = useState(0);
    const [currentPreset, setCurrentPreset] = useState(0);
    const [currentFxProgramIdx, setCurrentFxProgramIdx] = useState(0);
    const [presetSelected,setPresetSelected] = useState(true);
    const [showSaveDialog,setShowSaveDialog] = useState(false);
    const [deviceConnected,setDeviceConnected] = useState(false);

    function convertIndexedLEDColor(clrIndex)
    {
        if (clrIndex == 2)
        {
            return "#df3434";
        }
        else if (clrIndex == 3)
        {
            return "#f8bc18";
        }
        else if (clrIndex == 1)
        {
            return "#00FF00";
        }
        return "#000000";
    }

    function aboutHandler() {
        getAbout();
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
        if (newParameterValue !== null) {
            const cmdbfr = new ArrayBuffer(8);
            const cmd = new Uint8Array(cmdbfr);
            cmd[0] = USB_CMD_SET_PARAMETER;
            cmd[1] = 0;
            cmd[2] = 8;
            cmd[3] = 0;
            cmd[4] = newParameterValue.programIdx;
            cmd[5] = newParameterValue.parameterIdx;
            cmd[6] = newParameterValue.parameterVal & 0xFF;
            cmd[7] = (newParameterValue.parameterVal >> 8) & 0xFF;
            await writeIfPossible(cmd);
            console.log(`sending programIdx: ${newParameterValue.programIdx}, parameterIdx: ${newParameterValue.parameterIdx}, parameterVal: ${newParameterValue.parameterVal}`);
            //await Promise.race([readCommand(),new Promise(() => setTimeout(() => console.log("timeout"),5000))]);
            await readCommand();
            console.log("got response");
            newParameterValue = null;
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

    async function setPresetName(presetName)
    {
        const cmdbfr = new ArrayBuffer(presetName.length + 5);
        const cmd = new Uint8Array(cmdbfr);
        const encoder = new TextEncoder();
        const presetNameUint8 = encoder.encode(presetName);
        cmd[0] = USB_CMD_SET_PRESET_NAME;
        cmd[1] = 0;
        cmd[2] = presetName.length + 5;
        cmd[3] = 0;
        for (let c=0;c<presetNameUint8.length;c++)
        {
            cmd[c+4] = presetNameUint8[c];
        }
        cmd[presetName.length+4]=0;
        await writeIfPossible(cmd);
        presetNameChanging = 0;
    }

    async function setRouting(routing)
    {
        const cmdbfr = new ArrayBuffer(5);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_SET_ROUTING;
        cmd[1] = 0;
        cmd[2] = 5;
        cmd[3] = 0;
        cmd[4] = routing;
        await writeIfPossible(cmd);
    }

    async function setLedColor(clr)
    {
        const cmdbfr = new ArrayBuffer(5);
        const cmd = new Uint8Array(cmdbfr);
        cmd[0] = USB_CMD_SET_LED_COLOR;
        cmd[1] = 0;
        cmd[2] = 5;
        cmd[3] = 0;
        cmd[4] = clr;
        await writeIfPossible(cmd);
    }


    async function readCommand() {
        let transferResult;
        let isResolved = false;
        while (!isResolved) {
            if (currentCommandNr !== 0xFFFF) {
                const alignedSize = (Math.floor((currentCommandLength-1) / 64) + 1) * 64;
                transferResult = await ppfxDevice.transferIn(1, alignedSize);
            } else {
                transferResult = await ppfxDevice.transferIn(1, 64);
            }
            if (transferResult.status !== 'ok') {
                const data = new Uint8Array(transferResult.data.buffer);
                console.log(`readCommand(), returned unexpected status ${transferResult.status} when reading message header`);
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
                            //setPresets(presetBfr);
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
                            break;
                        }
                        default:
                            console.log(`unknown command ${commandBfr[0]} ${commandBfr[1]}`);
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
        if (newParameterValue == null) {
            newParameterValue ={ programIdx: currentFxProgramIdx, parameterIdx: position, parameterVal: value / 1 };
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
        setRouting(routingId);
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
        parameterValueSending = 0;
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
                    setDeviceConnected(true);
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
                    setDeviceConnected(true);
                    
                }).catch((error) => {
                    console.log(`Error: ${error}, No Device Found or selected`);
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
    }

    function handlePresetNameChange(presetname)
    {
        if (presetNameChanging === 0)
        {
            presetNameChanging = 1;
            const newpresets = presets.slice()
            newpresets[currentPreset].name = presetname; 
            setPresets(newpresets);
            setPresetName(presetname);
        }
    }
    
    function changeLEDColor(ledcolor)
    {
        const newpresets = presets.slice();
        newpresets[currentPreset].ledColorPreset = ledcolor;
        setLedColor(ledcolor);
        setPresets(newpresets);
    }

    function savePresetHandler()
    {   
        setShowSaveDialog(true);
        //savePreset(fxPrograms,presets[currentPreset]);
    }

    function showApplicationContent() {
        return (
            <>
                            <div className="editor-toppanel">
                    <div className="editor-vertical leftmost" style={{flexGrow: "1"}}>
                        <VolumeSlider onChange={(v) => handleMasterVolumeChange(v)}></VolumeSlider>
                        <div className="editor-vertical-label">Master Volume</div>
                    </div>
                    <div className="editor-vertical">
                        <label className="editor-switch on-root">
                            <input type="checkbox" checked={HiZOn}
                                onChange={(e) => handleHiZChange(e.target.checked)} />
                            <span className="editor-slider round"></span>
                        </label>
                        <div className="editor-toggle-label">HiZ</div>
                    </div>
                    <div className="editor-vertical">
                        <label className="editor-switch on-root">
                            <input type="checkbox" checked={MicOn}
                                onChange={(e) => handleMicChange(e.target.checked)} />
                            <span className="editor-slider round"></span>
                        </label>
                        <div className="editor-toggle-label">Mic</div>
                    </div>
                    
                </div>
                <div className="editor-panel">
                    <div className="editor-vertical leftmost">
                        <input 
                            id="currentPreset" 
                            className="editor-input editable" 
                            type="text" 
                            value={presets[currentPreset].name}
                            onChange={(e) => handlePresetNameChange(e.target.value) }
                        />
                        <div className="editor-vertical-label">Preset</div>
                    </div>
                    <div className="editor-vertical">
                        <div className='editor-effect-group-led editable' 
                            style={{ backgroundColor: convertIndexedLEDColor(presets[currentPreset].ledColorPreset)}}
                            onClick={() => {
                                let clridx;

                                    clridx = presets[currentPreset].ledColorPreset;
                                    clridx += 1;
                                    if (clridx > 3)
                                    {
                                        clridx = 1;
                                    }
                                    changeLEDColor(clridx);
                                          
                            }}> 
                        </div>
                        <div className="editor-vertical-label">LED Color</div>
                    </div>
                    <div className="editor-vertical">
                        <div id="currentPosition" className="editor-textfield">{currentPreset}</div>
                        <div className="editor-vertical-label">Position</div>
                    </div>
                    <div className="editor-vertical">
                        <div id="currentBank" className="editor-textfield">{currentBank}</div>
                        <div className="editor-vertical-label">Bank</div>
                    </div>
                    <button className="editor-button editable" 
                        style={{width: "90%", padding: "10px", margin: "14px"}}
                        onClick={savePresetHandler}>Save</button>
                    <span className="editor-filler"></span>
                </div>
                <div className="editor-panel" style={{maxHeight: "104px"}}>
                    <div className="editor-vertical-flex leftmost" style={{height: "stretch", justifyContent: "space-between"}}>
                        <button className="editor-button editable" 
                        style={{width: "90%", margin: "0"}}
                        onClick={previousBankHandler}>Previous Bank</button>
                        <button className="editor-button editable leftmost" 
                        style={{width: "90%",margin: "0"}}
                        onClick={nextBankHandler}>Next Bank</button>
                    </div>
                    <div className="editor-vertical-flex editable" style={{height: "stretch",justifyContent: "space-between", borderRadius: "4px"}}>
                        <input 
                            type="radio"   
                            name="presetNr" 
                            value="a" 
                            id="presetNr1" 
                            className="editor-text-selectable" 
                            checked={currentPreset === 0 && presetSelected} 
                            
                            onChange={() => {
                                setPresetSelected(true);
                                setCurrentPreset(0);
                                loadPreset(0);
                            }}></input>
                        <label htmlFor="presetNr1" id="presetNr1Label" className='preset-label'>{presets[0].name}</label>
                        <input 
                            type="radio" 
                            name="presetNr" 
                            value="b" 
                            id="presetNr2" 
                            className="editor-text-selectable" 
                            checked={currentPreset === 1 && presetSelected} 
                            style={{margin: '0',padding: '0', height: 'stretch'}}
                            onChange={() => {
                                setPresetSelected(true);
                                setCurrentPreset(1);
                                loadPreset(1);
                            }}></input>
                        <label htmlFor="presetNr2" id="presetNr2Label" className=' preset-label'>{presets[1].name}</label>
                        <input type="radio" name="presetNr" value="c" id="presetNr3" className="editor-text-selectable" checked={currentPreset === 2 && presetSelected} onChange={() => {
                            setPresetSelected(true);
                            setCurrentPreset(2);
                            loadPreset(2);
                        }}></input>
                        <label htmlFor="presetNr3" id="presetNr3Label" className=' preset-label'>{presets[2].name}</label>
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
            </>
        );
    }

    return (
        <>
            <div className="editor-main" id="editor_app">
                <div className="editor-title">PiPicoFX Editor</div>
            
                
            { deviceConnected ? showApplicationContent() : <button className="editor-button editable" onClick={requestDevice}>Connect device</button> }
            { showSaveDialog ? <SaveDialog 
                presets={presets} 
                bankNr={currentBank} 
                presetNr={currentPreset} 
                presetToSave={presets[currentPreset]}
                onCancel={() => setShowSaveDialog(false)}/> : null }
            </div>
        </>
    );
}

export default App;


