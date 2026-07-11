function ParametersDisplay(props)
{
    let id=0;
    let parametersContent;
    if (props.preset.programsAndParameters[props.effectIndex].programNr !== 0xff)
    { 
        parametersContent=props.fxPrograms[props.preset.programsAndParameters[props.effectIndex].programNr].parameterNames.map(p => {
            const internalId=id;
            return (
            <tr key={id} className="editor-horizontal">
                <td className="editor-vertical editor-effectparam">
                    <input type="range" name={p} min="0" max="4095" className="editor-param-slider" value={props.preset.programsAndParameters[props.effectIndex].parameters[id]} onChange={(e) => {
                        props.changeParameterValue(internalId,e.target.value);
                    } }/>
                    <div className="editor-vertical-label">{p}</div>
                </td>
                <td className="editor-paramvalue">{props.preset.programsAndParameters[props.effectIndex].displayNames[id++]}</td>
            </tr>
            );
        });
    }
    else
    {
        parametersContent="";
    }
    return (
        <table className="editor-vertical" id="parametersDisplay">
            {parametersContent}
        </table>
    );
}

export default ParametersDisplay