import { useState } from 'react';
import './App.css'

function EffectView(props)
{
    function getValue(effectState)
    {
        if (effectState === "on")
        {
            return 1;
        }
        else if (effectState === 'off')
        {
            return 0;
        }
    }

    function convertIdTLetter(id)
    {
        if (id == 0)
        {
            return "A";
        }
        else if (id == 1)
        {
            return "B";
        }
        else if (id == 2)
        {
            return "C";
        }
        return id;
    }

    return (
    <div className="editor-effect-group">
    <input type="radio" name="effectInFocus" value={"fx"+props.id} id={"effectView" + props.id} className="editor-text-selectable" onChange={(e) => {
            props.setCurrentFxProgramIdx(props.id);
        
    }}></input>
    <label htmlFor={"effectView" + props.id}>{"Effect " + convertIdTLetter(props.id)}</label>
    <label className="editor-switch">
        <input type="checkbox" checked={getValue(props.presets[props.currentPreset].programsAndParameters[props.id/1].state)} onChange={(e) => props.changeEffectState(props.id/1,e.target.checked)}/>
        <span className="editor-slider round"></span>
    </label>
      <EffectPrograms 
        id={props.id} 
        content={props.fxPrograms} 
        presets={props.presets}
        currentPreset={props.currentPreset}
        changePresets={props.changePresets}
        changeFxParams={props.changeFxParams} />
    </div>);
}

function EffectPrograms(props)
{
    const fxProgramContent = props.content.map( cnt => {
        if (cnt.freezable)
        {
            return (<option key={cnt.id} value={cnt.id} className='editor-fxprogram-freezable'>{cnt.name}</option>);
        }
        else
        {
            return (<option key={cnt.id} value={cnt.id}>{cnt.name}</option>);
        }
    });
    return (
        <>
			<select id={"fxSelector" + props.id}  value={props.presets[props.currentPreset].programsAndParameters[props.id/1].programNr}
             onChange={(e) => {
                const updatedPresets = props.presets.slice();
                updatedPresets[props.currentPreset].programsAndParameters[props.id/1].programNr = e.target.value/1;
                props.changePresets(props.id,e.target.value,updatedPresets);
            }}>
                {fxProgramContent}
			</select>
        </>
    );
}

export default EffectView