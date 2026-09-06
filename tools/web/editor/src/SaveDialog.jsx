import { useState } from 'react';
import { getBankContent,savePreset } from './App';
import './App.css'

function SaveDialog(props)
{
    const [bank,setBank] = useState(props.bankNr);
    const [savePresetNr,setSavePresetNr] = useState(props.presetNr);
    const [savePresets,setSavePresets] = useState(props.presets);


    return (
        <div className="editor-save-dialog-outer">
            <div  className="editor-save-dialog-inner">
                <div className='editor-vertical-flex'>
                    <div className='editor-panel'>
                        <div className="editor-vertical-flex leftmost" style={{height: "stretch", justifyContent: "space-between"}}>
                            <div style={{alignSelf: "center"}}>Bank {bank}</div>
                            <button className="editor-button editable" 
                            style={{width: "90%", margin: "0"}}
                            onClick={() => {
                                let nextbank;
                                if (bank > 0)
                                {
                                    nextbank = bank  - 1;
                                }
                                else 
                                {
                                    nextbank = 0;
                                }
                                setBank(nextbank);
                                setSavePresets([{name: ""},{name: ""},{name: ""}]);
                                getBankContent(nextbank).then(p => { 
                                    console.log(`setting presets`);
                                    console.log(p);
                                    setSavePresets(p);
                                });
                                }}>Previous Bank</button>
                            <button className="editor-button editable leftmost" 
                            style={{width: "90%",margin: "0"}}
                            onClick={() => {
                                let nextbank;
                                if (bank < 63)
                                {
                                    nextbank = bank + 1;
                                }
                                else 
                                {
                                    nextbank = 63;
                                }
                                setBank(nextbank);
                                setSavePresets([{name: ""},{name: ""},{name: ""}]);
                                getBankContent(nextbank).then(p => {
                                    console.log(`setting presets`);
                                    console.log(p);
                                    setSavePresets(p);
                                });
                                }}>Next Bank</button>
                        </div>
                        <div className="editor-vertical-flex editable" style={{height: "stretch",justifyContent: "space-between", borderRadius: "4px"}}>
                            <input 
                                type="radio"   
                                name="savePresetNr" 
                                value="a" 
                                id="savePresetNr1" 
                                className="editor-text-selectable" 
                                checked={savePresetNr === 0 } 
                                onChange={() => {
                                    setSavePresetNr(0);
                                }}></input>
                            <label htmlFor="savePresetNr1" id="savePresetNr1Label" className='preset-label'>1 ({savePresets[0].name})</label>
                            <input 
                                type="radio" 
                                name="savePresetNr" 
                                value="b" 
                                id="savePresetNr2" 
                                className="editor-text-selectable" 
                                checked={savePresetNr === 1} 
                                style={{margin: '0',padding: '0', height: 'stretch'}}
                                onChange={() => {
                                    setSavePresetNr(1);
                                }}></input>
                            <label htmlFor="savePresetNr2" id="savePresetNr2Label" className='preset-label'>2 ({savePresets[1].name})</label>
                            <input 
                                type="radio" 
                                name="savePresetNr" 
                                value="c" 
                                id="savePresetNr3" 
                                className="editor-text-selectable" 
                                checked={savePresetNr === 2} 
                                onChange={() => {
                                    setSavePresetNr(2);
                                }}></input>
                            <label htmlFor="savePresetNr3" id="savePresetNr3Label" className='preset-label'>3 ({savePresets[2].name})</label>
                        </div>
                    </div>
                    <div className='editor-panel'>
                        <button className='editor-button editable' onClick={props.onCancel}>Cancel</button>
                        <button className='editor-button editable' onClick={() => {
                            props.presetToSave.bankNr = bank;
                            props.presetToSave.presetNr = savePresetNr;
                            savePreset(props.presetToSave);
                            props.onCancel();
                        }} >Save</button>
                    </div>
                </div>
            </div>
        </div>
    )
}

export default SaveDialog;