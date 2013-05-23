#include "ToneGenBlock.h"
#include "ConstantBlock.h"
#include "OperatorBlock.h"
#include "LowPassBlock.h"
#include "FIRBlock.h"
#include "UnitImpulseBlock.h"
#include "SequenceBlock.h"
#include "HoldBlock.h"
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
void mid2freq(Generator* input,Generator* output);
float mixer_result;
std::vector<float> mid2freq_result;

int sample_rate=48000;

int main()
{

	std::vector<float> ir_vec;
	for(int i=0;i<=10;i++)
	{
		ir_vec.push_back(i/100.0);
	}
	for(int i=9;i>=0;i--)
	{
		ir_vec.push_back(i/100.0);
	}

	std::vector<int> notes;
	notes.push_back(65);
	notes.push_back(75);
	notes.push_back(-1);
	notes.push_back(72);
	notes.push_back(67);
	notes.push_back(67);
	notes.push_back(68);
	notes.push_back(-1);
	notes.push_back(65);
	notes.push_back(70);
	notes.push_back(72);
	notes.push_back(70);
	notes.push_back(65);

	notes.push_back(65);
	notes.push_back(-1);
	notes.push_back(-1);

	notes.push_back(65);
	notes.push_back(75);
	notes.push_back(-1);
	notes.push_back(72);
	notes.push_back(67);
	notes.push_back(67);
	notes.push_back(68);
	notes.push_back(65);
	notes.push_back(72);
	notes.push_back(75);
	notes.push_back(-1);
	notes.push_back(72);
	notes.push_back(77);

	int d=-1;

	std::vector<std::vector<void*> > seq(notes.size());
	for(unsigned i=0;i<notes.size();i++)
	{
		seq[i].push_back(&(notes[i]));
	}
	std::vector<void*> def;
	def.push_back(&d);

	SequenceBlock sb(seq,def);

    /*
       { "Blocks":
            { "cb":
                { "type": "ConstantBlock",
                  "inputs" : [],
                  "args" : [[440, 550]] },
              "tg":
                { "type": "ToneGenBlock",
                  "inputs" : [["cb", 0]],
                  "args" : ["saw"] },
              "lp":
                { "type": "LowPassBlock",
                  "inputs" : [["tg", 0]],
                  "args" : [0.9] },
              "mixer":
                { "type": "OperatorBlock",
                  "inputs" : [["lg", 0]],
                  "args" : ["mix"] }
            }
        }
            
       */

	OperatorBlock mid2freqb(&mid2freq);
	HoldBlock hb(0.3*sample_rate);

	LowPassBlock lb(0.999);

	ToneGenBlock tg(&saw);

	//FIRBlock fb(ir_vec);

	mid2freqb.inputs.push_back(sb.outputs[0]);
	hb.inputs.push_back(mid2freqb.outputs[0]);
	lb.inputs.push_back(hb.outputs[0]);
	tg.inputs.push_back(lb.outputs[0]);
	//fb.inputs.push_back(tg.outputs[0]);

	sb.start();
	mid2freqb.start();
	hb.start();
	lb.start();
	tg.start();
	//fb.start();

	Generator* g=tg.outputs[0];

	writeWav(g,"out.wav",1,sample_rate,sample_rate*15);

	for(int i=0;i<10;i++)
	{
		g->get();
		vectorPrinter(*g);
		cout << endl;
	}

/*
    PortaudioBlock pa = PortaudioBlock(sample_rate);

    pa.inputs.push_back(mixer.outputs[0]);
    pa.start();
*/

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

void mid2freq(Generator* input,Generator* output)
{
	unsigned len=input->size();
	if(output->size() != len)
	{
		output->resize(len);
		mid2freq_result.resize(len);
	}

	for(unsigned i=0;i<len;i++)
	{
		int val = *((int*)(input->at(i)));
		if(val==-1) // Null
		{
			(*output)[i]=0;
		}
		else
		{
			mid2freq_result[i]=440*pow(2,((val-69)/12.0))/sample_rate;
			(*output)[i]=&(mid2freq_result[i]);
		}
	}
}

