
function ParametersDisplay(props)
{
    const parametersContent=props.parameters.map(p => {
        return (
        <div key={p.id} className="editor-horizontal">
			<div className="editor-vertical editor-effectparam">
				<input type="range" name={p.name} min="0" max="4095" />
				<div className="editor-vertical-label">{p.displayName}</div>
			</div>
			<div className="editor-paramvalue">{p.value}</div>
		</div>
        );
    });
    return (
        <div className="editor-vertical" id="parametersDisplay">
            {parametersContent}
        </div>
    );
}

export default ParametersDisplay