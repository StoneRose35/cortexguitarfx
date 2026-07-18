import rt_serial_256x256 from './../assets/rt_serial_256x256.png';
import rt_parallel_256x256 from './../assets/rt_parallel_256x256.png';
import rt_s_2p_256x256 from './../assets/rt_s_2p_256x256.png';
import rt_2p_sl_256x256 from './../assets/rt_2p_sl_256x256.png';
import rt_2s_p_256x256 from './../assets/rt_2s_p_256x256.png';
import './App.css'
import { useState } from 'react';

function Routing(props)
{
    const [routingId,setRoutingId] = useState(props.presets[props.currentPreset].routing);
    const routingImages=[rt_serial_256x256,rt_parallel_256x256,rt_s_2p_256x256,rt_2p_sl_256x256,rt_2s_p_256x256];
    function updateRoutingId()
    {
        if (routingId == 4)
        {
            setRoutingId(() => 0);
            props.changeRouting(0);
        }
        else
        {
            setRoutingId((routingId) => routingId + 1);
            props.changeRouting(routingId + 1);
        }
    }

    return (
        <img src={routingImages[props.presets[props.currentPreset].routing]} className='editor-routing' onClick={() => updateRoutingId()}></img>
    );

}

export default Routing;