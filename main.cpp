#include "ToneGenBlock.h"
#include "ConstantBlock.h"
#include "OperatorBlock.h"
#include "PortaudioBlock.h"
#include "write_file.h"

#include <iostream>
#include <math.h>
#define PI 3.14159265

using namespace std;

void vectorPrinter(const std::vector<void*> &v);
float sine(float t);
float saw(float t);

void mix(Generator* input,Generator* output);
float mixer_result;

int main()
{
	int sample_rate=48000;

	float freq=440.0/sample_rate;
	float freq2=550.0/sample_rate;
	std::vector<void*> c;
	c.push_back(&freq);
	c.push_back(&freq2);

	ConstantBlock cb(c);
	ToneGenBlock tg(&saw);
	OperatorBlock mixer(&mix);

	tg.inputs.push_back(cb.outputs[0]);
	mixer.inputs.push_back(tg.outputs[0]);

	cb.start();
	tg.start();
	mixer.start();

	Generator* g=mixer.outputs[0];

	//writeWav(g,"out.wav",1,sample_rate,sample_rate*3);

	for(int i=0;i<10;i++)
	{
		g->get();
		vectorPrinter(*g);
		cout << endl;
	}

    PortaudioBlock pa = PortaudioBlock(sample_rate);

    pa.inputs.push_back(mixer.outputs[0]);
    pa.start();

	return 0;
}

float sine(float t)
{
	return sin(t*2*PI);
}

float saw(float t)
{
	return 2*t-1;
}
void vectorPrinter(const std::vector<void*> &v)
{
	cout << '[';
	for(unsigned i=0;i<v.size();i++)
	{
		float* f=(float*)(v[i]);
		if(f==0) // null ptr
		{
			cout << "None";
		}
		else
		{
			cout << *f;
		}
		if(i<v.size()-1)
		{
			cout << ", ";
		}
	}
	cout << ']';
}

void mix(Generator* input,Generator* output)
{
	if(output->size()!=1)
	{
		output->resize(1);
	}

	float acc=0;
	int n=0;

	for(unsigned i=0;i<input->size();i++)
	{
		if(input->at(i)==0) // Null
		{
			continue;
		}
		acc+=*(float*)(input->at(i));
		n++;
	}
	if(n==0)
	{
		(*output)[0]=0; // Null pointer
	}

	mixer_result=acc/n;
	(*output)[0]=&mixer_result;
}

