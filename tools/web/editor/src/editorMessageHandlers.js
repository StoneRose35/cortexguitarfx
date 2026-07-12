export function processAboutMessage(msg, idx, size, appendToConsole) {
    appendToConsole(String.fromCharCode(...msg.slice(idx, idx + size)));
    return idx + size;
}

export function processGetPrograms(msg, msgIdx, size) {
    const nPrograms = msg[msgIdx + 4];
    const fxProgs = new Array(nPrograms);
    let idx = msgIdx + 5;
    let progCnt = 0;

    while (idx < msgIdx + size) {
        const fxProg = {
            ParameterCount: msg[idx] & 0x7F,
            freezable: false,
            name: '',
            id: progCnt,
            parameterNames: []
        };

        if ((msg[idx] & 0x80) > 0) {
            fxProg.freezable = true;
        }

        idx += 1;
        const nameArray = [];
        while (msg[idx] !== 0 && idx < msgIdx + size) {
            nameArray.push(msg[idx++]);
        }
        fxProg.name = String.fromCharCode(...nameArray);
        fxProgs[progCnt++] = fxProg;
        idx += 1;
    }

    const fxProg = {
            ParameterCount: 0,
            freezable: false,
            name: '<None>',
            id: 63,
            parameterNames: []
        };
    fxProgs[progCnt++] = fxProg;

    return { fxProgs, nextIdx: msgIdx + size };
}

export function processInputAndMasterVolume(msg, msgIdx, size) {
    return {
        hiZOn: ((msg[msgIdx + 4] & 1) >> 0) === 1,
        micOn: ((msg[msgIdx + 4] & 2) >> 1) === 1,
        masterVolume: msg[msgIdx + 5],
        nextIdx: msgIdx + size
    };
}

export function processGetParameterNames(msg, msgIdx, size, fxProgs) {
    const parameterNamesLocal = [];
    let idx = msgIdx + 5;

    while (idx < msgIdx + size) {
        const nameArray = [];
        while (msg[idx] !== 0 && idx < msg.length) {
            nameArray.push(msg[idx++]);
        }
        idx += 1;
        parameterNamesLocal.push(String.fromCharCode(...nameArray));
    }

    const updatedFxProgs = fxProgs.slice();
    updatedFxProgs[msg[msgIdx + 4]].parameterNames = parameterNamesLocal;
    return { fxProgs: updatedFxProgs, nextIdx: msgIdx + size };
}

export function processCurrentBankAndPresetNr(msg, msgIdx, size) {
    return {
        bank: msg[msgIdx + 4],
        preset: msg[msgIdx + 5],
        nextIdx: msgIdx + size
    };
}


export function processParameterDisplayValue(msg, msgIdx, size, presetBfr, currentPreset) {
    let idx = msgIdx + 6;
    const nameArray = [];
    while (msg[idx] !== 0 && idx < msgIdx + size) {
        nameArray.push(msg[idx++]);
    }
    idx += 1;

    const effectIndex = msg[msgIdx + 4];
    const parameterIdx = msg[msgIdx + 5];
    const updatedPresets = presetBfr.slice();
    updatedPresets[currentPreset].programsAndParameters[effectIndex].displayNames[parameterIdx] = String.fromCharCode(...nameArray);

    return { presetBfr: updatedPresets, nextIdx: msgIdx + size };
}
