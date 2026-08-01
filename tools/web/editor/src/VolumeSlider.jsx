
import { useState,useRef } from 'react';

let isclicked=false;

function VolumeSlider(props)
{
    let [paramName,setParamName]=useState("Master Volume");
    const [value,setValue]=useState(props.initialValue/2.55 + "%");
    const clickPos=useRef(0);
    const containerElement=useRef(null);
    const currentWidth=useRef(0);

    //setValue(props.initialValue);
    return (
        <div className="parameter-slider-container editable" style={{ minHeight: '24px', marginBottom: '0', marginLeft: '0', paddingLeft: '0'}} >
            <div className='parameter-slider-container-inner'>
                <div className="parameter-slider-bar" style={{ width: value }} >

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
                    console.log("current width in %: " + currentWidth.current);
                }} 
                onPointerUp={(e) => {
                    console.log("click released");
                    //console.log("moved " + (e.clientX-clickPos.current)/containerElement.current.clientWidth + " along x"); 
                    isclicked = false;
                }}
                onPointerMove={(e) => {
                    if (isclicked == true)
                    {
                        //console.log("setting value to " + (currentWidth.current+(e.nativeEvent.offsetX - clickPos.current)/containerElement.current.clientWidth*100)/1 + "%");
                        setValue( (currentWidth.current+(e.nativeEvent.offsetX - clickPos.current)/containerElement.current.clientWidth*100)/1 + "%");
                        props.onChange(Math.round(e.nativeEvent.offsetX/containerElement.current.clientWidth*4096));
                    }
                }}
            >
            </div> 
        </div>
    )
}

export default VolumeSlider;