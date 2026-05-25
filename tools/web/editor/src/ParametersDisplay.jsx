function ParametersDisplay(props)
{
    let id=0;
    let parametersContent;
    if (props.preset.programsAndParameters[props.effectIndex].programNr !== 0xff)
    { 
        parametersContent=props.fxPrograms[props.preset.programsAndParameters[props.effectIndex].programNr].parameterNames.map(p => {
            const internalId=id;
            return (
            <div key={id} className="editor-horizontal">
                <div className="editor-vertical editor-effectparam">
                    <input type="range" name={p} min="0" max="4095" value={props.preset.programsAndParameters[props.effectIndex].parameters[id]} onChange={(e) => {
                        props.changeParameterValue(internalId,e.target.value);
                    } }/>
                    <div className="editor-vertical-label">{p}</div>
                </div>
                <div className="editor-paramvalue">{props.preset.programsAndParameters[props.effectIndex].parameters[id++]}</div>
            </div>
            );
        });
    }
    else
    {
        parametersContent="";
    }
    return (
        <div className="editor-vertical" id="parametersDisplay">
            {parametersContent}
        </div>
    );
}

export default ParametersDisplay