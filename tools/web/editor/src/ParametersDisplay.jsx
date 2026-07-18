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
                <>
            <tr key={id} className="editor-horizontal">
                <td className="editor-vertical editor-effectparam">
                    <ParameterSlider id={id} preset={props.preset} parameterName={p} parameterValue={props.preset.programsAndParameters[props.effectIndex].displayNames[id++]} onChange={(v) => props.changeParameterValue(internalId,v)}/>
                </td>
            </tr>
            
            </>
            );
        });
    }
    else
    {
        parametersContent="";
    }
    return (
        <table className="editor-vertical" id="parametersDisplay">
            <tbody>
            {parametersContent}
            </tbody>
        </table>
    );
}

export default ParametersDisplay