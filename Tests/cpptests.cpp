#include <iostream>
#include "stdint.h"

#define FIRST_CNT_VALUE 12
#define SECOND_CNT_VALUES 17
using namespace std;
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

int main(int argc,char ** argv)
{
    DerivedA * aInst = new DerivedA();
    float res;
    for(uint16_t c=0;c<20;c++)
    {
        res = aInst->process((float)c);
        cout << "process of " << (float)c << " is " << res << endl;
    }
}