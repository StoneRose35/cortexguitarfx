
import { useState,useRef } from 'react';

let isclicked=false;

function ParameterSlider(props)
{
    let [paramName,setParamName]=useState("Volume");
    const [value,setValue]=useState(props.initialValue/40.96 + "%");
    const clickPos=useRef(0);
    const containerElement=useRef(null);
    const currentWidth=useRef(0);

    //setValue(props.initialValue);
    return (
        <div className="parameter-slider-container editable" >
            <div className='parameter-slider-container-inner'>
                <div className="parameter-slider-bar" style={{ width: props.preset.programsAndParameters[props.effectIndex].parameters[props.id]/40.96 + "%"}} >

                </div>
            </div>
            <div className="parameter-slider-name" >
                {props.parameterName}
            </div>
            <div className="parameter-slider-displayvalue">
                {props.parameterValue}
            </div>

            <div className="parameter-slider-overlay"
                ref={containerElement}
                onPointerDown={(e) => {
                    clickPos.current = e.nativeEvent.offsetX;
                    isclicked = true;
                    currentWidth.current = clickPos.current/containerElement.current.offsetWidth*100;
                    //console.log("point down event on " + paramName);
                }} 
                onTouchStart={(e) => {
                    //console.log("started touch event on " + paramName);
                    clickPos.current = e.nativeEvent.touches.item(0).clientX;
                    isclicked = true;
                }}
                onPointerUp={(e) => {
                    //console.log("click released");
                    //console.log("moved " + (e.clientX-clickPos.current)/containerElement.current.clientWidth + " along x"); 
                    isclicked = false;
                }}
                onPointerMove={(e) => {
                    if (isclicked == true)
                    {
                        //console.log("pointer move event on " + paramName);
                        //console.log("setting value to " + (currentWidth.current+(e.nativeEvent.offsetX - clickPos.current)/containerElement.current.clientWidth*100)/1 + "%");
                        setValue( (currentWidth.current+(e.nativeEvent.offsetX - clickPos.current)/containerElement.current.clientWidth*100)/1 + "%");
                        props.onChange(Math.round(e.nativeEvent.offsetX/containerElement.current.clientWidth*4096));
                    }
                }}
                onTouchMove={(e) => {
                    if (isclicked == true)
                    {
                        
                        setValue( (currentWidth.current+(e.nativeEvent.touches.item(0).clientX - clickPos.current)/containerElement.current.clientWidth*100)/1 + "%");
                        props.onChange(Math.round(e.nativeEvent.touches.item(0).clientX/containerElement.current.clientWidth*4096));
                        //console.log("touch move event on " + paramName);
                    }
                }}
            >
            </div> 
        </div>
    )
}

export default ParameterSlider;