export function createInitialFxPrograms() {
    return [
        {
            id: 0,
            name: 'AmpModel',
            freezable: false,
            parameterNames: ['Gain', 'Tone', 'Presence', 'Volume']
        },
        {
            id: 1,
            name: 'Delay',
            freezable: true,
            parameterNames: ['Time', 'Feedback', 'Mix']
        },
        {
            id: 2,
            name: 'Reverb',
            freezable: true,
            parameterNames: ['Time', 'Mix']
        },
        {
            id: 255,
            name: '<None>',
            freezable: false,
            parameterNames: []
        }
    ];
}

export function createInitialPresets() {
    return [
        {
            bankNr: 1,
            presetNr: 0,
            routing: 0,
            name: 'B0 P0',
            ledColorPreset: 1,
            ledColorA: 1,
            ledColorB: 2,
            ledColorC: 3,
            programsAndParameters: [
                {
                    programNr: 0,
                    parameters: [50, 100, 150, 200],
                    displayNames: ['50', '100', '150', '200'],
                    state: 'on'
                },
                {
                    programNr: 1,
                    parameters: [400, 500, 600],
                    displayNames: ['400', '500', '600'],
                    state: 'off'
                },
                {
                    programNr: 255,
                    parameters: [700, 800, 900],
                    displayNames: ['700', '800', '900'],
                    state: 'on'
                }
            ]
        },
        {
            bankNr: 1,
            presetNr: 1,
            routing: 0,
            name: 'B0 P1',
            ledColorPreset: 2,
            ledColorA: 2,
            ledColorB: 1,
            ledColorC: 3,
            programsAndParameters: [
                {
                    programNr: 0,
                    parameters: [200, 150, 100, 50],
                    displayNames: ['200', '150', '100', '50'],
                    state: 'off'
                },
                {
                    programNr: 2,
                    parameters: [400, 500, 600],
                    displayNames: ['400', '500', '600'],
                    state: 'on'
                },
                {
                    programNr: 255,
                    parameters: [700, 800, 900],
                    displayNames: ['700', '800', '900'],
                    state: 'on'
                }
            ]
        },
        {
            bankNr: 1,
            presetNr: 2,
            routing: 0,
            name: 'B0 P2',
            ledColorPreset: 3,
            ledColorA: 3,
            ledColorB: 2,
            ledColorC: 1,
            programsAndParameters: [
                {
                    programNr: 0,
                    parameters: [200, 150, 100, 50],
                    displayNames: ['200', '150', '100', '50'],
                    state: 'on'
                },
                {
                    programNr: 2,
                    parameters: [400, 500, 600],
                    displayNames: ['400', '500', '600'],
                    state: 'on'
                },
                {
                    programNr: 1,
                    parameters: [700, 800, 900],
                    displayNames: ['700', '800', '900'],
                    state: 'off'
                }
            ]
        }
    ];
}
