import './App.css'
import { useState } from 'react';



function EffectView(props)
{
    
    return (
    <div className="editor-effect-group">
    <input type="radio" name="effectInFocus" value={"fx"+props.id} id={"effectView" + props.id} className="editor-text-selectable"></input><label htmlFor={"effectView" + props.id}>{"Effect " + props.id}</label>
    <label className="editor-switch">
        <input type="checkbox" />
        <span className="editor-slider round"></span>
    </label>
      <EffectPrograms id={props.id + "selector"} content={props.fxPrograms}/>
    </div>);
}

function EffectPrograms(props)
{
    const fxProgramContent = props.content.map( cnt => {
        if (cnt.freezable)
        {
            return (<option key={cnt.id} className='editor-fxprogram-freezable'>{cnt.name}</option>);
        }
        else
        {
            return (<option key={cnt.id} >{cnt.name}</option>);
        }
    });
    return (
			<select id={props.id}>
                {fxProgramContent}
			</select>
    );
}

export default EffectView