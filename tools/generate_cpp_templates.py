import os.path
import os
import json
import re
import copy
rootpath = os.path.abspath(os.path.dirname(__file__))
header_folder = "../Inc/pipicofx"
cpp_files_folder = "../Src/pipicofx"

json_example = """ {"fxPrograms":
[{
 "displayName": "theBestEffect",
 "className": "tBE",
 "parameters": [{
        "name": "Parameter 2",
        "control": 1,
        "rawValue": 0,
        "increment": 2    
    },
    {
        "name": "Parameter 3",
        "control": 1,
        "rawValue": 0,
        "increment": 512    
    }    
    ]
}]}
"""

template_hpp_head = """
#pragma once
#include "FxProgram.hpp"
extern "C" {{
#include <stdint.h>
#include "stringFunctions.h"
#include "audio/gainstage.h"
#include "picofxCore.h"
}}

namespace PiPicoFX {{
    namespace {class_name} {{
        class {class_name} : public FxProgram
        {{
            public:
                {class_name}() : FxProgram({n_parameters},"{display_name}"){{
                    this->setup();
                }};
                int16_t processSample(int16_t);
                GainStageDataType presetVolume={{
                    .gain=0xff,
                    .offset=0
                }};
            private:
                void setup();
        }};
"""

template_hpp_parameter = """
        class Param{param_nr}:  public FxProgramParameter
        {{
            public:
                Param{param_nr}({class_name}* p) :FxProgramParameter({control},"{parameter_name}")
                {{
                    rawValue = {raw_value};
                    increment = {increment};
                    pData=p;
                }};
                void parameterCallback(uint16_t val);
                void parameterDisplay(char* chrbfr);
            private:
                {class_name} * pData;
        }};"""

template_hpp_tail = """
    }
}
"""

template_cpp_head = """
#include "pipicofx/{header_file_name}"
extern "C" {{
#include "stringFunctions.h"
#include "audio/gainstage.h"
}}
using namespace PiPicoFX;

int16_t {class_name}::{class_name}::processSample(int16_t sampleIn)
{{
    // TODO implement audio processor
    return 0;
}}

void {class_name}::{class_name}::setup()
{{
    // TODO complete setup function
{params_space}
}}
"""


template_cpp_param = """
void {class_name}::Param{param_nr}::parameterCallback(uint16_t val)
{{
    // TODO affect the state of the fx program in any way based on 'val'
    this->rawValue = val; 
}}

void {class_name}::Param{param_nr}::parameterDisplay(char*res)
{{
    // TODO put some content descripting the actual parameter value in res
}}"""

class CppFileGeneratorException(Exception): 
    def __init__(self,message=""):
        self.msg = message
    
        

def filesExist(className): 
    folderPath = os.path.join(rootpath,header_folder)
    foundHeader=False
    for f in os.listdir(folderPath):
        if os.path.isfile(os.path.join(folderPath,f)) and len(f) > 8:
            cut_filename = f.split(os.path.pathsep)[-1][4:-4]
            if cut_filename == className:
                foundHeader = True
    foundClass=False
    folderPath = os.path.join(rootpath,cpp_files_folder)
    for f in os.listdir(folderPath):
        if os.path.isfile(os.path.join(folderPath,f)) and len(f) > 8:
            cut_filename = f.split(os.path.pathsep)[-1][4:-4]
            if cut_filename == className:
                foundClass = True
    if foundClass != foundHeader:
        msg = "found only Header or only Class file for {}".format(className)
        raise CppFileGeneratorException(msg)
    else:
        return foundClass
    
def getNextFileNumber():
    current_prog_nr = 0
    folderPath = os.path.join(rootpath,header_folder)
    for f in os.listdir(folderPath):
        m = re.search("([0-9]{3})_[a-zA-Z0-9]*\\.hpp",f)
        if m is not None:
            progNr = int(m.group(1))
            if current_prog_nr < progNr:
                current_prog_nr = progNr
    return current_prog_nr+1

def getFileNames(nr,class_name):
    return "{:03}_{}.hpp".format(nr,class_name), "{:03}_{}.cpp".format(nr,class_name)


def generateClassFile(json_input):
    try:
        jsonData = json.loads(json_input)
    except:
        with open(json_input) as input_file:
            try: 
                jsonData = json.load(input_file)
            except:
                raise CppFileGeneratorException("input is neither a json string nor a path to a json file")
    for fxp in jsonData["fxPrograms"]:
        classname = fxp["className"]
        if not filesExist(classname):
            nr = getNextFileNumber()
            number_of_params = len(fxp["parameters"])
            header_filename,class_filename = getFileNames(nr,classname)
            hpp_full_filename = os.path.join(rootpath,header_folder,header_filename)
            with open(hpp_full_filename,"wt") as f:
                headerpart = copy.deepcopy(template_hpp_head)
                headerpart = headerpart.format(class_name = classname,n_parameters = number_of_params,display_name=fxp["displayName"])
                f.write(headerpart)
                for c in range(number_of_params):
                    param = copy.deepcopy(template_hpp_parameter)
                    param = param.format(class_name = classname,
                                         param_nr = c+1,
                                         control = fxp["parameters"][c]["control"],
                                         parameter_name = fxp["parameters"][c]["name"],
                                         raw_value = fxp["parameters"][c]["rawValue"],
                                         increment = fxp["parameters"][c]["increment"]
                                         )
                    f.write(param)
                f.write(template_hpp_tail)
            cpp_full_filename = os.path.join(rootpath,cpp_files_folder,class_filename)
            with open(cpp_full_filename,"wt") as f:
                paramsspace = ""
                for c in range(number_of_params):
                    paramsspace += "    this->addParameter(new Param{}(this));\n".format(c+1)

                cpp_headerpart = copy.deepcopy(template_cpp_head)

                cpp_headerpart = cpp_headerpart.format(class_name=classname,header_file_name = header_filename, params_space=paramsspace)
                f.write(cpp_headerpart)
                for c in range(number_of_params):
                    param_implementation = copy.deepcopy(template_cpp_param)
                    param_implementation = param_implementation.format(class_name = classname,param_nr=c+1)
                    f.write(param_implementation)
                pass




if __name__ == "__main__":
    generateClassFile(os.path.join(rootpath,"fxPrograms.json"))
