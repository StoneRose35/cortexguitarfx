#include <iostream>
#include "stdint.h"

#define FIRST_CNT_VALUE 12
#define SECOND_CNT_VALUES 17
using namespace std;


extern "C" {
#include "math.h"
void zeroString(char*data,int16_t len)
{
    for (uint16_t c=0;c<len;c++)
    {
        *(data+c)=0;
    }
}

/*
float int2float(int32_t a)
{
    return (float)a;
}

int32_t float2int(float a)
{
    return (int32_t)a;
}

float fln(float a)
{
    return logf(a);
}
*/

float convolve(const float*coeffs,float*data,uint32_t offset)
{
    float res=0.0f;
    for(uint32_t c=0;c<64;c++)
    {
        res += *(coeffs + c) * *(data + ((offset + c)&0x3F));
    }
    return res;
}

}

class BaseClass
{
    public:
        float process(float a)
        {
            this->cnt ++;
            if (this->cnt == FIRST_CNT_VALUE)
            {
                onFirst();
            }
            if (this-> cnt == SECOND_CNT_VALUES)
            {
                onSecond();
            }
            if(this->cnt > SECOND_CNT_VALUES)
            {
                this->cnt=0;
            }
            cout << "BaseClass::process()" << endl;
            return a*4.534f;
        };
        virtual void onFirst()
        {
            cout << "BaseClass::onFirst()" << endl;
        };
        virtual void onSecond()
        {
            cout << "BaseClass::onSecond()" << endl;
        };

        private:
            uint16_t cnt=0;
};

class DerivedA: public BaseClass
{
    public:
        float process(float a)
        {
            a = BaseClass::process(a);
            a = a-1.0f;
            return a;
        };

        void onFirst() override
        {
            cout << "DerivedA::onFirst()" << endl;
        };

        void onSecond() override
        {
            cout << "DerivedA::onSecond()" << endl;
        }
};

class SimpleClass
{
    public:
        float a;
        float b;
        char name[24];
};

class ClassWithDynamicArray
{
    public:
        SimpleClass ** dynamicArray;
        int arrayLength;
        ClassWithDynamicArray(int len)
        {
            this->dynamicArray = new SimpleClass*[len];
            this->arrayLength = len;
            for (uint8_t c=0;c<len;c++)
            {
                this->dynamicArray[c]=nullptr;
            }
        }
};


void dynamicArrayClassTest()
{
        ClassWithDynamicArray cls1=ClassWithDynamicArray(4);
    SimpleClass simpleClass = SimpleClass();
    simpleClass.a = 45.22f;
    simpleClass.b = 12.21f;
    sprintf(simpleClass.name,"Funk;");
    cls1.dynamicArray[2]=&simpleClass;
    for (uint8_t c=0;c< 4;c++)
    {
        if (cls1.dynamicArray[c] == nullptr)
        {
            printf("entry at %d is nullptr\r\n",c);
        }
        else
        {
            printf("\ta: %f\r\n",cls1.dynamicArray[c]->a);
            printf("\tb: %f\r\n",cls1.dynamicArray[c]->b);
            printf("\tname: %s\r\n",cls1.dynamicArray[c]->name);
        }
    }
    printf("initiated dynamic object\n");
}

void examinePointerWrapping()
{
    uint16_t p1,p2;
    uint16_t deltap;
    p1 = 500;
    p2 = 504;
    for (uint8_t c=0;c<25;c++)
    {
        deltap = (p2++ - p1++)&0x1FF;
        printf("p1: %d, p2: %d, deltap: %d\r\n",p1,p2,deltap);
        p2 &= 0x1FF;
        p1 &= 0x1FF;
    }

}

int main(int argc,char ** argv)
{
    examinePointerWrapping();
}
