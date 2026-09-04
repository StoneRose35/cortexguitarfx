import ParameterSlider from "./ParameterSlider";

function ParametersDisplay(props)
{
    let id=0;
    let parametersContent;
    if (props.preset.programsAndParameters[props.effectIndex].programNr < 63)
    { 
        parametersContent=props.fxPrograms[props.preset.programsAndParameters[props.effectIndex].programNr].parameterNames.map(p => {
            const internalId=id;
            return (
            <tr key={id} className="editor-horizontal">
                <td className="editor-vertical editor-effectparam">
                    <ParameterSlider 
                        id={id} 
                        preset={props.preset} 
                        effectIndex={props.effectIndex}
                        parameterName={p} 
                        initialValue={props.preset.programsAndParameters[props.effectIndex].parameters[id]}
                        parameterValue={props.preset.programsAndParameters[props.effectIndex].displayNames[id++]} 
                        onChange={(v) => props.changeParameterValue(internalId,v)}/>
                </td>
            </tr>
            );
        });
            return (
        <table className="editor-vertical" id="parametersDisplay" style={{padding: "0"}}>
            <tbody>
            {parametersContent}
            </tbody>
        </table>
    );
    }
    else
    {
            return (
        <table className="editor-vertical" id="parametersDisplay" style={{padding: "0"}}>
        </table>
    );
    }

}

export default ParametersDisplay