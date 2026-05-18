

function Console(props)
{
    return (
    <div className="editor-console" style={{height: 156 + 'px'}}>
		<div className="editor-console-inner" id="editor-console">
            {props.content}
		</div>
	</div>
    );
}

export default Console;