#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include "mathext.h"
#include "linxaudio.h"

#define min(x,y) ((x) < (y) ? (x) : (y))
//#define max(x,y) ((x) > (y) ? (x) : (y))

//pCB->MessageBox("FSM Kick XP 1.0SP1\r\n\r\nIf you like this machine, consider supporting some people\r\n""that are doing something more useful than coding Buzz machines\r\n(like FSF, Debian, Xiph, EFF, ACLU, Amnesty Intl etc)");

struct linx_pin kickxp_pins[] = {
	{ "Pitch limit", linx_pin_type_in_scalar_float, 1.0f, 96.0f, 1.0f, 1.0f },
	{ "Trigger", linx_pin_type_in_scalar_float, 0.0, 2.0f, 0.0f, 1.0f },
	{ "Start", linx_pin_type_in_scalar_float, 1.0f, 240.0f, 145.0f, 1.0f },
	{ "End", linx_pin_type_in_scalar_float, 1.0f, 240.0f, 50.0f, 1.0f },
	{ "Buzz", linx_pin_type_in_scalar_float, 0.0f, 100.0, 55.0f, 1.0f },
	{ "Click", linx_pin_type_in_scalar_float, 0.0f, 100.0, 28.0f, 1.0f },
	{ "Punch", linx_pin_type_in_scalar_float, 0.0f, 100.0, 47.0f, 1.0f },
	{ "ToneDecay", linx_pin_type_in_scalar_float,  1.0f, 240.0f, 30.0f, 1.0f },
	{ "ToneShape", linx_pin_type_in_scalar_float,  1.0f, 240.0f, 27.0f, 1.0f },
	{ "BDecay", linx_pin_type_in_scalar_float,  1.0f, 240.0f, 55.0f, 1.0f },
	{ "CDecay", linx_pin_type_in_scalar_float,  1.0f, 240.0f, 55.0f, 1.0f },
	{ "DecSlope", linx_pin_type_in_scalar_float,  1.0f, 240.0f, 1.0f, 1.0f },
	{ "DecTime", linx_pin_type_in_scalar_float,  1.0f, 240.0f, 32.0f, 1.0f },
	{ "RelSlope", linx_pin_type_in_scalar_float, 1.0f, 240.0f, 105.0f, 1.0f },
	{ "NoteDelay", linx_pin_type_in_scalar_float, 0.0f, 12.0f, 0.0f, 1.0f },
	{ "Out", linx_pin_type_out_buffer_float },
};

static float thumpdata1[1024];

struct kickxp_tap {
	float PitchLimit;
	float ThisPitchLimit;
	float StartFrq;
	float ThisStartFrq;
	float EndFrq;
	float ThisEndFrq;
	float TDecay;
	float ThisTDecay;
	float TShape;
	float ThisTShape;
	float DSlope;
	float ThisDSlope;
	float DTime;
	float ThisDTime;
	float RSlope;
	float ThisRSlope;
	float BDecay;
	float ThisBDecay;
	float CDecay;
	float ThisCDecay;
	float CurVolume;
	float ThisCurVolume;
	float LastValue;
	float AntiClick;
	float ClickAmt;
	float PunchAmt;
	float BuzzAmt;
	float Amp;
	float DecAmp;
	float BAmp;
	float MulBAmp;
	float CAmp;
	float MulCAmp;
	float Frequency;
	int SamplesToGo;
	int Retrig;
	int RetrigCount;

	double xSin, xCos, dxSin, dxCos;

	int EnvPhase;
	int LeftOver;
	int Age;
	double OscPhase;
};
 
struct kickxp {
	float *Buffer;
	int Pos;
	float DryOut;
	short *thump1;
	int thump1len;
	float samplerate;
	struct kickxp_tap tap;
};

void Trigger(struct kickxp_tap* pt)
{
	if (pt->Retrig && pt->RetrigCount>0)
	{
		pt->SamplesToGo = pt->Retrig;
		pt->RetrigCount--;
	} else
		pt->SamplesToGo = 0;
	pt->AntiClick = pt->LastValue;
	pt->EnvPhase = 0;
	pt->OscPhase = pt->ClickAmt;
	pt->LeftOver = 0;
	pt->Age = 0;
	pt->Amp = 32;
	pt->ThisPitchLimit = pt->PitchLimit;
	pt->ThisDTime = pt->DTime;
	pt->ThisDSlope = pt->DSlope;
	pt->ThisRSlope = pt->RSlope;
	pt->ThisBDecay = pt->BDecay;
	pt->ThisCDecay = pt->CDecay;
	pt->ThisTDecay = pt->TDecay;
	pt->ThisTShape = pt->TShape;
	pt->ThisStartFrq = pt->StartFrq;
	pt->ThisEndFrq = pt->EndFrq;
	pt->ThisCurVolume = pt->CurVolume;
}

void kickxp_tap_tick(struct kickxp_tap *pt, float samplerate, struct linx_value_array* in_values) {
	int i;
	bool bTrig = false;
	int ndelay = -1;
	bool bNote = false;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_index == 0) {
			float note = v->floatvalue;
			//v = (v & 15) - 1 + 12 * (v >> 4);
			pt->PitchLimit = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
			bTrig = true;
			bNote = true;
		} else if (v->pin_index == 1) {
			// trigger
			pt->CurVolume = v->floatvalue;// *(32000.0f / 128.0f);
			bTrig = true;
		} else if (v->pin_index == 2) {
			pt->StartFrq = 33.0f * powf(128, v->floatvalue / 240.0f);
		} else if (v->pin_index == 3) {
			pt->EndFrq = 33.0f * powf(16, v->floatvalue / 240.0f);
		} else if (v->pin_index == 4) {
			pt->BuzzAmt = 3.0f * v->floatvalue / 100.0f;
		} else if (v->pin_index == 5) {
			pt->ClickAmt = v->floatvalue / 100.0f;
		} else if (v->pin_index == 6) {
			pt->PunchAmt = v->floatvalue / 100.0f;
		} else if (v->pin_index == 7) {
			pt->TDecay = (v->floatvalue / 240.0f) * ((1.0f / 400.0f) * (44100.0f / samplerate));
		} else if (v->pin_index == 8) {
			pt->TShape = v->floatvalue / 240.0f;
		} else if (v->pin_index == 9) {
			pt->BDecay = v->floatvalue / 240.0f;
		} else if (v->pin_index == 10) {
			pt->CDecay = v->floatvalue / 240.0f;
		} else if (v->pin_index == 11) {
			pt->DSlope = pow(20, v->floatvalue / 240.0f - 1.0f) * 25.0f / samplerate;
		} else if (v->pin_index == 12) {
			pt->DTime = v->floatvalue * samplerate / 240.0;
		} else if (v->pin_index == 13) {
			pt->RSlope = powf(20.0, v->floatvalue / 240.0f - 1.0f) * 25.0f / samplerate;
		} else if (v->pin_index == 14) {
			ndelay = (int)v->floatvalue;
		}
	}

/* TODO: handle note offs - interpret trigger 0 as noteoff?
  if (ptval->pitchlimit == NOTE_OFF && pt->EnvPhase<pt->ThisDTime)
  {
    if (ptval->ndelay && ptval->ndelay<6)
      pt->ThisDTime=pt->EnvPhase+ptval->ndelay * samplerate / 6.0f;
    else
      pt->ThisDTime=pt->EnvPhase;
  }*/

	if (bTrig) {
		if (ndelay != -1) {
			pt->Retrig = 0;
			if (ndelay < 6) {
				pt->SamplesToGo = ndelay * samplerate / 6.0f;
			} else if (ndelay < 11) {
				Trigger(pt);
				pt->SamplesToGo = (ndelay - 5) * samplerate / 6.0f;
			} else {
				Trigger(pt);
				pt->Retrig = pt->SamplesToGo = (ndelay - 10) * samplerate / 6.0f;
				pt->RetrigCount = 6 / (ndelay - 10) - 2;
			}
		} else {
			pt->Retrig = 0;
			Trigger(pt);
		}
	}
}

void kickxp_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct kickxp* data = (struct kickxp*)malloc(sizeof(struct kickxp));
	memset(data, 0, sizeof(struct kickxp));
	plugin->data = data;

	for (int i = 0; i < 1024; i++) {
		thumpdata1[i] = sin(1.37f * i + 0.1337f * (1024 - i) * sin(1.1f * i)) * pow(1.0f/256.0f, i / 1024.0f);
	}

	data->samplerate = (float)samplerate;
	data->tap.CurVolume = 1.0f;// 32000.0;
	data->tap.Age = 0;
	data->tap.Amp = 0;
	data->tap.LeftOver = 32000;
	data->tap.EnvPhase = 6553600;
}

bool kickxp_tap_work(struct kickxp_tap* trk, int samplerate, float *pout, int c) {

	trk->OscPhase = fmod(trk->OscPhase, 1.0);
	float Ratio = trk->ThisEndFrq / trk->ThisStartFrq;
	if (trk->AntiClick < -64000) trk->AntiClick = -64000;
	if (trk->AntiClick >= 64000) trk->AntiClick = 64000;
	int i = 0;
	double xSin = trk->xSin, xCos = trk->xCos;
	double dxSin = trk->dxSin, dxCos = trk->dxCos;
	float LVal = 0;
	float AClick = trk->AntiClick;
	float Amp = trk->Amp;
	float DecAmp = trk->DecAmp;
	float BAmp = trk->BAmp;
	float MulBAmp = trk->MulBAmp;
	float CAmp = trk->CAmp;
	float MulCAmp = trk->MulCAmp;
	float Vol = 0.5 * trk->ThisCurVolume;
	bool amphigh = Amp >= 16;
	int Age = trk->Age;

	float odsr = 1.0f / samplerate;
	while (i < c) {
		if (trk->SamplesToGo == 1) {
			Trigger(trk);
			AClick = trk->AntiClick;
			Age = trk->Age;
			Amp = trk->Amp;
		}
		if (trk->LeftOver <= 0) {
			trk->LeftOver = 32;
			double EnvPoint = trk->EnvPhase * trk->ThisTDecay;
			double ShapedPoint = pow(EnvPoint, trk->ThisTShape * 2.0);
			trk->Frequency = (float)(trk->ThisStartFrq * pow((double)Ratio, ShapedPoint));
			if (trk->Frequency > 10000.f) trk->EnvPhase = 6553600;
			if (trk->EnvPhase < trk->ThisDTime) {
			trk->DecAmp = DecAmp = trk->ThisDSlope;
			trk->Amp= Amp = (float)(1 - DecAmp * trk->EnvPhase);
		} else {
			DecAmp = trk->ThisDSlope;
			Amp = (float)(1 - DecAmp * trk->ThisDTime);
			if (Amp > 0) {
				trk->DecAmp = DecAmp = trk->ThisRSlope;
				trk->Amp = Amp = Amp - DecAmp * (trk->EnvPhase - trk->ThisDTime);
			}
		}
		if (trk->Amp <= 0) {
			trk->Amp = 0;
			trk->DecAmp = 0;
			if (fabs(AClick) < 0.00012f && !trk->SamplesToGo) {
				return amphigh;
			}
		}

		trk->BAmp = BAmp = trk->BuzzAmt * (float)(pow(1.0f / 256.0f, trk->ThisBDecay * trk->EnvPhase * (odsr * 10)));
		float CVal = (float)(pow(1.0f / 256.0f, trk->ThisCDecay * trk->EnvPhase * (odsr * 20)));
		trk->CAmp = CAmp = trk->ClickAmt * CVal;
		trk->Frequency *= (1 + 2 * trk->PunchAmt * CVal * CVal * CVal);
		if (trk->Frequency > 10000) trk->Frequency = 10000;
		if (trk->Frequency < trk->ThisPitchLimit) trk->Frequency = trk->ThisPitchLimit;

		trk->MulBAmp = MulBAmp = (float)pow(1.0f / 256.0f, trk->ThisBDecay * (10 * odsr));
		trk->MulCAmp = MulCAmp = (float)pow(1.0f / 256.0f, trk->ThisCDecay * (10 * odsr));
		xSin = (float)sin(2.0 * 3.141592665 * trk->OscPhase);
		xCos = (float)cos(2.0 * 3.141592665 * trk->OscPhase);
		dxSin = (float)sin(2.0 * 3.141592665 * trk->Frequency / samplerate);
		dxCos = (float)cos(2.0 * 3.141592665 * trk->Frequency / samplerate);
		LVal = 0.0;
		trk->dxSin = dxSin, trk->dxCos = dxCos;
	}
	int max = min(i + trk->LeftOver, c);
	if (trk->SamplesToGo > 0) {
		max = min(max, i + trk->SamplesToGo - 1);
	}
	if (Amp > 0.00001f && Vol > 0) {
		amphigh = true;
		float OldAmp = Amp;
		if (BAmp > 0.01f) {
			for (int j = i; j < max; j++) {
				LVal = (AClick + Amp * Vol * xSin);
				pout[j] += LVal;
				if (xSin > 0) {
					float D = (float)(Amp * Vol * BAmp * xSin * xCos);
					pout[j] -= D;
					LVal -= D;
				}
				double xSin2 = (xSin * dxCos + xCos * dxSin);
				double xCos2 = (xCos * dxCos - xSin * dxSin);
				xSin = xSin2; 
				xCos = xCos2;
				Amp -= DecAmp;
				BAmp *= MulBAmp;
				AClick *= 0.98f;
			}
		} else for (int j = i; j < max; j++) {
			LVal = (AClick + Amp * Vol * xSin);
			pout[j] += LVal;
			double xSin2 = (xSin * dxCos + xCos * dxSin);
			double xCos2 = (xCos * dxCos - xSin * dxSin);
			xSin = xSin2; 
			xCos = xCos2;
			Amp -= DecAmp;
			AClick *= 0.98f;
		}
		if (fabs(AClick) < 0.0001f) AClick = 0.0001f;
			if (OldAmp > 0.1f && CAmp > 0.001f) {
				int max2 = i + min(max - i, 1024 - Age);
				float LVal2 = 0.f;
				for (int j = i; j < max2; j++) {
					pout[j] += (LVal2 = OldAmp * Vol * CAmp * thumpdata1[Age]);
					OldAmp -= DecAmp;
					CAmp *= MulCAmp;
					Age++;
				}
				LVal += LVal2;
			}
		}
		if (Amp) {
			trk->OscPhase += (max - i) * trk->Frequency / samplerate;
			trk->EnvPhase += max - i;
			trk->LeftOver -= max - i;
		} else {
			trk->LeftOver = 32000;
		}
		if (trk->SamplesToGo > 0) trk->SamplesToGo -= max - i;
		i = max;
	}
	trk->xSin = xSin, trk->xCos = xCos;
	trk->LastValue = LVal;
	trk->AntiClick = AClick;
	trk->Amp = Amp;
	trk->BAmp = BAmp;
	trk->CAmp = CAmp;
	trk->Age = Age;
	return amphigh;
}

void kickxp_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	struct kickxp* data = (struct kickxp*)self->data;
	struct linx_buffer* out_buffer = &pin_buffers[15];

	kickxp_tap_tick(&data->tap, data->samplerate, in_values);
	float* psamples = out_buffer->float_buffer;

	for (int i = 0; i < sample_count; i++)
		psamples[i] = 0.0;

	if (kickxp_tap_work(&data->tap, data->samplerate, psamples, sample_count)) {
		out_buffer->write_count = 1;
	}
}

void kickxp_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
	case 12: // delay time
		snprintf(result_name, result_name_size, "%0.2f s", value / 240.0f);
		break;
	}
}

void kickxp_destroy(struct linx_vertex_instance* plugin) {
	struct kickxp* data = (struct kickxp*)plugin->data;
	free(data);
}

struct linx_factory kickxp_factory = {
	sizeof(struct linx_factory),  "KickXP", 0,
	(sizeof(kickxp_pins) / sizeof(struct linx_pin)), kickxp_pins,
	0, 0,
	kickxp_create, kickxp_destroy, kickxp_process, kickxp_describe_value
};

struct linx_factory* kickxp_get_factory() {
	return &kickxp_factory;
}
