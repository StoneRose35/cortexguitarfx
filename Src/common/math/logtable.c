#include "stdint.h"

__attribute__ ((section (".qspi_data")))
const float logtable[]= {

-120.000000000f, -28.164490963f, -22.144002226f, -18.622214103f, -16.123457901f, -14.185268758f, -12.601651249f, -11.262720751f, -10.102885782f, -9.079838421f, -8.164691081f, -7.336839399f, -6.581069866f, -5.885829166f, -5.242136720f, -4.642873311f, 
-4.082299766f, -3.555721809f, -3.059250861f, -2.589629594f, -2.144102285f, -1.720316833f, -1.316249593f, -0.930146928f, -0.560479217f, -0.205904249f, 0.134762195f, 0.462570202f, 0.778455252f, 1.083254309f, 1.377719190f, 1.662527733f, 
1.938293199f, 2.205572219f, 2.464871564f, 2.716653924f, 2.961342876f, 3.199327175f, 3.430964467f, 3.656584526f, 3.876492069f, 4.090969241f, 4.300277786f, 4.504660966f, 4.704345267f, 4.899541900f, 5.090448151f, 5.277248573f, 
5.460116064f, 5.639212822f, 5.814691217f, 5.986694566f, 6.155357832f, 6.320808271f, 6.483165998f, 6.642544516f, 6.799051194f, 6.952787698f, 7.103850389f, 7.252330685f, 7.398315397f, 7.541887029f, 7.683124060f, 7.822101202f, 
7.958889638f, 8.093557237f, 8.226168764f, 8.356786056f, 8.485468208f, 8.612271721f, 8.737250660f, 8.860456790f, 8.981939701f, 9.101746932f, 9.219924083f, 9.336514916f, 9.451561455f, 9.565104075f, 9.677181588f, 9.787831324f, 
9.897089203f, 10.004989806f, 10.111566443f, 10.216851210f, 10.320875052f, 10.423667814f, 10.525258294f, 10.625674292f, 10.724942653f, 10.823089315f, 10.920139343f, 11.016116974f, 11.111045647f, 11.204948046f, 11.297846121f, 11.389761130f, 
11.480713661f, 11.570723661f, 11.659810466f, 11.747992822f, 11.835288907f, 11.921716361f, 12.007292299f, 12.092033337f, 12.175955608f, 12.259074783f, 12.341406087f, 12.422964315f, 12.503763852f, 12.583818682f, 12.663142408f, 12.741748263f, 
12.819649122f, 12.896857521f, 12.973385661f, 13.049245424f, 13.124448385f, 13.199005819f, 13.272928714f, 13.346227780f, 13.418913458f, 13.490995928f, 13.562485120f, 13.633390720f, 13.703722180f, 13.773488723f, 13.842699351f, 13.911362854f, 
13.979487814f, 14.047082613f, 14.114155440f, 14.180714294f, 14.246766992f, 14.312321175f, 14.377384310f, 14.441963701f, 14.506066486f, 14.569699650f, 14.632870023f, 14.695584288f, 14.757848985f, 14.819670514f, 14.881055137f, 14.942008988f, 
15.002538070f, 15.062648262f, 15.122345322f, 15.181634891f, 15.240522494f, 15.299013544f, 15.357113347f, 15.414827102f, 15.472159905f, 15.529116753f, 15.585702544f, 15.641922082f, 15.697780076f, 15.753281148f, 15.808429830f, 15.863230569f, 
15.917687726f, 15.971805585f, 16.025588347f, 16.079040136f, 16.132165000f, 16.184966916f, 16.237449784f, 16.289617438f, 16.341473642f, 16.393022092f, 16.444266419f, 16.495210192f, 16.545856915f, 16.596210032f, 16.646272927f, 16.696048928f, 
16.745541303f, 16.794753267f, 16.843687979f, 16.892348545f, 16.940738021f, 16.988859410f, 17.036715665f, 17.084309693f, 17.131644352f, 17.178722454f, 17.225546763f, 17.272120003f, 17.318444852f, 17.364523944f, 17.410359873f, 17.455955193f, 
17.501312416f, 17.546434016f, 17.591322428f, 17.635980051f, 17.680409245f, 17.724612336f, 17.768591612f, 17.812349329f, 17.855887709f, 17.899208938f, 17.942315173f, 17.985208537f, 18.027891122f, 18.070364989f, 18.112632170f, 18.154694667f, 
18.196554452f, 18.238213470f, 18.279673637f, 18.320936843f, 18.362004951f, 18.402879796f, 18.443563190f, 18.484056916f, 18.524362736f, 18.564482385f, 18.604417576f, 18.644169996f, 18.683741311f, 18.723133163f, 18.762347174f, 18.801384942f, 
18.840248043f, 18.878938034f, 18.917456450f, 18.955804807f, 18.993984599f, 19.031997301f, 19.069844371f, 19.107527244f, 19.145047340f, 19.182406058f, 19.219604782f, 19.256644875f, 19.293527685f, 19.330254542f, 19.366826759f, 19.403245633f, 
19.439512444f, 19.475628458f, 19.511594922f, 19.547413071f, 19.583084122f, 19.618609279f, 19.653989730f, 19.689226649f, 19.724321197f, 19.759274519f, 19.794087747f, 19.828761999f, 19.863298382f, 19.897697986f, 19.931961891f, 19.966091164f, 

};

__attribute__ ((section (".qspi_data")))
const float linvalues[] = {
0.000001000f, 0.039063887f, 0.078126773f, 0.117189660f, 0.156252547f, 0.195315434f, 0.234378320f, 0.273441207f, 0.312504094f, 0.351566980f, 0.390629867f, 0.429692754f, 0.468755641f, 0.507818527f, 0.546881414f, 0.585944301f, 
0.625007188f, 0.664070074f, 0.703132961f, 0.742195848f, 0.781258734f, 0.820321621f, 0.859384508f, 0.898447395f, 0.937510281f, 0.976573168f, 1.015636055f, 1.054698941f, 1.093761828f, 1.132824715f, 1.171887602f, 1.210950488f, 
1.250013375f, 1.289076262f, 1.328139148f, 1.367202035f, 1.406264922f, 1.445327809f, 1.484390695f, 1.523453582f, 1.562516469f, 1.601579355f, 1.640642242f, 1.679705129f, 1.718768016f, 1.757830902f, 1.796893789f, 1.835956676f, 
1.875019563f, 1.914082449f, 1.953145336f, 1.992208223f, 2.031271109f, 2.070333996f, 2.109396883f, 2.148459770f, 2.187522656f, 2.226585543f, 2.265648430f, 2.304711316f, 2.343774203f, 2.382837090f, 2.421899977f, 2.460962863f, 
2.500025750f, 2.539088637f, 2.578151523f, 2.617214410f, 2.656277297f, 2.695340184f, 2.734403070f, 2.773465957f, 2.812528844f, 2.851591730f, 2.890654617f, 2.929717504f, 2.968780391f, 3.007843277f, 3.046906164f, 3.085969051f, 
3.125031938f, 3.164094824f, 3.203157711f, 3.242220598f, 3.281283484f, 3.320346371f, 3.359409258f, 3.398472145f, 3.437535031f, 3.476597918f, 3.515660805f, 3.554723691f, 3.593786578f, 3.632849465f, 3.671912352f, 3.710975238f, 
3.750038125f, 3.789101012f, 3.828163898f, 3.867226785f, 3.906289672f, 3.945352559f, 3.984415445f, 4.023478332f, 4.062541219f, 4.101604105f, 4.140666992f, 4.179729879f, 4.218792766f, 4.257855652f, 4.296918539f, 4.335981426f, 
4.375044313f, 4.414107199f, 4.453170086f, 4.492232973f, 4.531295859f, 4.570358746f, 4.609421633f, 4.648484520f, 4.687547406f, 4.726610293f, 4.765673180f, 4.804736066f, 4.843798953f, 4.882861840f, 4.921924727f, 4.960987613f, 
5.000050500f, 5.039113387f, 5.078176273f, 5.117239160f, 5.156302047f, 5.195364934f, 5.234427820f, 5.273490707f, 5.312553594f, 5.351616480f, 5.390679367f, 5.429742254f, 5.468805141f, 5.507868027f, 5.546930914f, 5.585993801f, 
5.625056688f, 5.664119574f, 5.703182461f, 5.742245348f, 5.781308234f, 5.820371121f, 5.859434008f, 5.898496895f, 5.937559781f, 5.976622668f, 6.015685555f, 6.054748441f, 6.093811328f, 6.132874215f, 6.171937102f, 6.210999988f, 
6.250062875f, 6.289125762f, 6.328188648f, 6.367251535f, 6.406314422f, 6.445377309f, 6.484440195f, 6.523503082f, 6.562565969f, 6.601628855f, 6.640691742f, 6.679754629f, 6.718817516f, 6.757880402f, 6.796943289f, 6.836006176f, 
6.875069063f, 6.914131949f, 6.953194836f, 6.992257723f, 7.031320609f, 7.070383496f, 7.109446383f, 7.148509270f, 7.187572156f, 7.226635043f, 7.265697930f, 7.304760816f, 7.343823703f, 7.382886590f, 7.421949477f, 7.461012363f, 
7.500075250f, 7.539138137f, 7.578201023f, 7.617263910f, 7.656326797f, 7.695389684f, 7.734452570f, 7.773515457f, 7.812578344f, 7.851641230f, 7.890704117f, 7.929767004f, 7.968829891f, 8.007892777f, 8.046955664f, 8.086018551f, 
8.125081438f, 8.164144324f, 8.203207211f, 8.242270098f, 8.281332984f, 8.320395871f, 8.359458758f, 8.398521645f, 8.437584531f, 8.476647418f, 8.515710305f, 8.554773191f, 8.593836078f, 8.632898965f, 8.671961852f, 8.711024738f, 
8.750087625f, 8.789150512f, 8.828213398f, 8.867276285f, 8.906339172f, 8.945402059f, 8.984464945f, 9.023527832f, 9.062590719f, 9.101653605f, 9.140716492f, 9.179779379f, 9.218842266f, 9.257905152f, 9.296968039f, 9.336030926f, 
9.375093812f, 9.414156699f, 9.453219586f, 9.492282473f, 9.531345359f, 9.570408246f, 9.609471133f, 9.648534020f, 9.687596906f, 9.726659793f, 9.765722680f, 9.804785566f, 9.843848453f, 9.882911340f, 9.921974227f, 9.961037113f, 

};

__attribute__ ((section (".qspi_code")))
float fastlog(float x)
{
    uint32_t c=0;
    if (x < linvalues[0])
    {
        return logtable[0];
    }
    if (x > linvalues[255])
    {
        return logtable[255];
    }
    while(linvalues[c] < x)
    {
        c++;
    }
    return (logtable[c] - logtable[c-1])/(linvalues[c] - linvalues[c-1])*(x-linvalues[c-1]);
}
