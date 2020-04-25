/*
	phatman.c - C port of Krzysztof Foltman's LGPL Phatman Buzz plugin

	Copyright (C) 2001-2007 Krzysztof Foltman  <kfoltman@users.sourceforge.net>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public License
	as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
*/
#define _USE_MATH_DEFINES
#define NOMINMAX
#define _POSIX_
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "linxaudio.h"
#include "inertia.h"

#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))

#define MAX_DELAY   8192
#define DELAY_MASK  8191
#define TABLE_BITS 15
#define COUNTER_BITS 28
#define TABLE_SIZE (1<<TABLE_BITS)
#define FRACT_RANGE (1<<(COUNTER_BITS-TABLE_BITS))

inline int f2i(double d) {
#ifdef WIN32
	const double magic = 6755399441055744.0; // 2^51 + 2^52
	double tmp = d + magic;
	return *(int*) &tmp;
#else
    // poor man's solution :)
    return (int)rint(d);
#endif
}

inline float INTERPOLATE(float pos, float start, float end) {
  return ((start)+(pos)*((end)-(start)));
}

struct linx_pin phatman_pins[] = {
	{ "InL", linx_pin_type_in_buffer_float },
	{ "InR", linx_pin_type_in_buffer_float },
	{ "OutL", linx_pin_type_out_buffer_float },
	{ "OutR", linx_pin_type_out_buffer_float },
	{ "DryOut", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 1.0f, 0.01f },
	{ "Phase", linx_pin_type_in_scalar_float, -3.14f, 3.14f, 0.0f, 0.01f },
	{ "MinDelay", linx_pin_type_in_scalar_float, 0.1f, 10.0f, 2.0f, 0.01f },
	{ "Modulation", linx_pin_type_in_scalar_float, 0.1f, 10.0f, 1.0f, 0.01f },
	{ "LFO Rate", linx_pin_type_in_scalar_float, 0.050f, 32.0f, 0.065f, 0.001f },
	{ "WetOut", linx_pin_type_in_scalar_float, 0.0f, 1.0f, 1.0f, 0.01f },
	{ "Feedback", linx_pin_type_in_scalar_float, -1.0f, 1.0f, 0.0f, 0.01f },
	{ "StereoPhasing", linx_pin_type_in_scalar_float, -3.14f, 3.14f, -1.57f, 0.01f },
	{ "LFOShape", linx_pin_type_in_scalar_int, 0.0f, 7.0f, 5.0f, 1.0f }
};

struct phatman {
	float dry_out;
	double phase;
	float min_delay;
	float mod_depth;
	float lfo_rate;
	float wet_out;
	float feedback;
	float stereo_phasing;
	int lfo_shape;
	float samplerate;
	float LastPos, LastPos2;
    //double Phase;
	float vsin,vcos,dsin,dcos,psin,pcos;
	
    float DeltaPhase;
    int Pos;
    float *Buffer;
    float FeedbackLimiter;
    float FuncTable[8*TABLE_SIZE];

};

void phatman_process_chunk(struct phatman* data, struct linx_buffer* pin_buffers, int offset, int sample_count) {
	int i;
	struct linx_buffer* in_left = &pin_buffers[0];
	struct linx_buffer* in_right = &pin_buffers[1];
	struct linx_buffer* out_left = &pin_buffers[2];
	struct linx_buffer* out_right = &pin_buffers[3];
	float* pData = data->Buffer;
    int nPos = data->Pos & DELAY_MASK;

    float pos0 = data->min_delay + data->mod_depth * 0.5f;
    float dpos = data->mod_depth * 0.5f;

    float FB = data->feedback * data->FeedbackLimiter;
    float WO = data->wet_out;
    float pos_left, pos_right;
	float acpos_left, acpos_right;
	float floatPos;
	int intPos;

	if (data->lfo_shape) {
        int nPhase = (int)((1<<COUNTER_BITS)*data->phase/(2*M_PI));
        int nDPhase = (int)((1<<COUNTER_BITS)*data->DeltaPhase/(2*M_PI));
        int nPhaseShift = (int)(TABLE_SIZE*data->stereo_phasing/(2*M_PI));
		int nShift = TABLE_SIZE*(data->lfo_shape-1);

        int nPhasePos = nPhase>>(COUNTER_BITS-TABLE_BITS);
        float fracPhase = (float)((nPhase&(FRACT_RANGE-1))*(1.0/FRACT_RANGE));
        float d0 = data->FuncTable[nShift+(nPhasePos&(TABLE_SIZE-1))];
        float d1 = data->FuncTable[nShift+((nPhasePos+1)&(TABLE_SIZE-1))];
        pos_left = pos0 + dpos * (d0 + (d1 - d0) * fracPhase);

        nPhasePos += nPhaseShift; // przesunicie fazy
        d0 = data->FuncTable[nShift + ((nPhasePos)&(TABLE_SIZE-1))];
        d1 = data->FuncTable[nShift + ((nPhasePos+1)&(TABLE_SIZE-1))];
        pos_right = pos0 + dpos * (d0 + (d1 - d0) * fracPhase);

        acpos_left = data->LastPos - pos_left;
        acpos_right = data->LastPos2 - pos_right;
        pos_left += acpos_left;
		pos_right += acpos_right;

		for (i = 0; i < sample_count; i ++) {
			float delayed_left, delayed_right;
			float in_left_value, in_right_value;
			float fracPhase;
            int nPhasePos = nPhase>>(COUNTER_BITS-TABLE_BITS);
            nPhase += nDPhase;
            
			fracPhase = (float)(nPhase&(FRACT_RANGE - 1)) * (1.0 / FRACT_RANGE);
            d0 = data->FuncTable[nShift + (nPhasePos&(TABLE_SIZE - 1))];
            d1 = data->FuncTable[nShift + ((nPhasePos + 1)&(TABLE_SIZE - 1))];
            pos_left = pos0 + dpos * (d0 + (d1 - d0) * fracPhase) + acpos_left;

            nPhasePos += nPhaseShift; // przesunicie fazy
            d0 = data->FuncTable[nShift + ((nPhasePos)&(TABLE_SIZE-1))];
            d1 = data->FuncTable[nShift + ((nPhasePos + 1)&(TABLE_SIZE-1))];
            pos_right = pos0 + dpos * (d0 + (d1 - d0) * fracPhase) + acpos_right;

            acpos_left *= 0.9995f;
			acpos_right *= 0.9995f;
            
			floatPos = nPos - pos_left;
            intPos = (int)(floatPos);

            delayed_left = INTERPOLATE(floatPos - intPos, pData[intPos&DELAY_MASK], pData[(intPos+1)&DELAY_MASK]);

            floatPos = nPos - pos_right;
            intPos = (int)(floatPos);

			delayed_right = INTERPOLATE(floatPos - intPos, pData[intPos&DELAY_MASK], pData[(intPos+1)&DELAY_MASK]);

			in_left_value = in_left->float_buffer[offset + i];
			in_right_value = in_right->float_buffer[offset + i];
            pData[nPos] = 0.5f * (in_left_value + in_right_value + (delayed_left + delayed_right) * FB);
			out_left->float_buffer[offset + i] = in_left_value * data->dry_out + delayed_left * WO;
			out_right->float_buffer[offset + i] = in_right_value * data->dry_out + delayed_right * WO;

            nPos = (nPos + 1) & DELAY_MASK;
        }
    } else {
        float vsin = data->vsin, vcos=data->vcos, dsin=data->dsin, dcos=data->dcos, psin=data->psin, pcos=data->pcos;
        pos_left = (float)(pos0 + dpos * vsin);
        pos_right = (float)(pos0 + dpos * (psin * vcos + pcos * vsin));

        acpos_left = data->LastPos - pos_left;
        acpos_right = data->LastPos2 - pos_right;
        pos_left += acpos_left;
		pos_right += acpos_right;

        for (i = 0; i < sample_count; i ++) {
			float delayed_left, delayed_right;
			float in_left_value = 0, in_right_value = 0;
			float vsin1, vcos1;

            pos_left = (float)(pos0 + dpos * vsin + acpos_left);
            pos_right = (float)(pos0 + dpos * (psin * vcos + pcos * vsin) + acpos_right);
            acpos_left *= 0.9995f;
			acpos_right *= 0.9995f;
            
			vsin1 = vsin * dcos + vcos * dsin;
			vcos1 = vcos * dcos - vsin  *dsin;
            vsin = vsin1; 
			vcos = vcos1;

            floatPos = nPos - pos_left;
            intPos = f2i(floatPos);
            delayed_left = INTERPOLATE(floatPos - intPos, pData[intPos&DELAY_MASK], pData[(intPos + 1)&DELAY_MASK]);

			floatPos = nPos - pos_right;
            intPos = f2i(floatPos);

            delayed_right = INTERPOLATE(floatPos - intPos, pData[intPos&DELAY_MASK], pData[(intPos + 1)&DELAY_MASK]);

			if (in_left->write_count > 0) {
				in_left_value = in_left->float_buffer[offset + i];
			}
			if (in_right->write_count > 0) {
				in_right_value = in_right->float_buffer[offset + i];
			}
            pData[nPos] = 0.5f * (in_left_value + in_right_value + (delayed_left + delayed_right) * FB);
			out_left->float_buffer[offset + i] = in_left_value * data->dry_out + delayed_left * WO;
			out_right->float_buffer[offset + i] = in_right_value * data->dry_out + delayed_right * WO;

			nPos = (nPos + 1) & DELAY_MASK;
        }
        data->vsin=vsin, data->vcos=vcos;
        data->dsin=dsin, data->dcos=dcos;
        data->psin=psin, data->pcos=pcos;
    }
    data->phase = fmod(data->phase + sample_count * data->DeltaPhase, 2 * M_PI);
    data->LastPos = pos_left;
    data->LastPos2 = pos_right;

	out_left->write_count = 1;
	out_right->write_count = 1;
}

void phatman_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct phatman* data = (struct phatman*)self->data;
	int so = 0;
	int maxs = 128;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			// DryOut=(float)pow(2.0,(gval.dryout/10.0-24.0)/6.0);
			data->dry_out = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 5) {
			//pt->Phase=(float)(2*PI*gval.resetphase/240);
			// 0..2pi
			data->phase = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 6) {
			// pt->MinDelay = (float)(pMasterInfo->SamplesPerSec * ptval->mindelay/10000.0);
			data->min_delay = data->samplerate * v->floatvalue / 1000.0f;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 7) {
			// pt->ModDepth = (float)(pMasterInfo->SamplesPerSec * ptval->moddepth/10000.0);
			data->mod_depth = data->samplerate * v->floatvalue / 1000.0f;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 8) {
			data->lfo_rate = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 9) {
			// pt->WetOut = ptval->wetout?(float)pow(2.0,(ptval->wetout/10.0-24.0)/6.0):(float)0.0;
			data->wet_out = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 10) {
			// pt->Feedback = (float)((ptval->feedback - 64) * (1.0 / 64.0)); 
			data->feedback = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 11) {
			// pt->StereoPhasing = (float)((ptval->stereophasing - 64) * PI * (1.0 / 64.0)); 
			data->stereo_phasing = v->floatvalue;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 12) {
			data->lfo_shape = v->intvalue;
		}
	}

	// preparetrack(), only for !lfo_shape
	if (!data->lfo_shape) {
		data->vsin=(float)sin(data->phase);
		data->vcos=(float)cos(data->phase);
		data->dsin=(float)sin(data->DeltaPhase);
		data->dcos=(float)cos(data->DeltaPhase);
		data->psin=(float)sin(data->stereo_phasing);
		data->pcos=(float)cos(data->stereo_phasing);
	}

	data->FeedbackLimiter=(float)((data->feedback > 0.995f) ? (0.995f / data->feedback) : 0.995f);

	// chunk for delay ring buffer
	data->Pos &= DELAY_MASK;
    while (so < sample_count) {
        int end = min(so+maxs, sample_count);

		phatman_process_chunk(data, pin_buffers, so, end - so);
        data->Pos = (data->Pos + end - so)&DELAY_MASK;
        so = end;
    }

}

void phatman_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
}

void phatman_create(struct linx_vertex_instance* plugin, int samplerate) {
    int i;
	float slope = 12.0f;
	float accum = 0.0;
	struct phatman* data = (struct phatman*)malloc(sizeof(struct phatman));
	memset(data, 0, sizeof(struct phatman));
	data->Buffer = (float*)malloc(sizeof(float) * MAX_DELAY);
	memset(data->Buffer, 0, sizeof(float) * MAX_DELAY);
	data->samplerate = (float)samplerate;
	
    for (i = 0; i < TABLE_SIZE; i++) {
        accum = accum * 0.8 + 0.2 * ((rand() & 255) / 256.0) * (i < TABLE_SIZE / 2 ? 1 : (1 - (i - TABLE_SIZE / 2) / (TABLE_SIZE / 2.0)));
        data->FuncTable[i+0*TABLE_SIZE] = (i<TABLE_SIZE/2)?(i*4.0/TABLE_SIZE-1):(3-i*4.0/TABLE_SIZE);
        data->FuncTable[i+1*TABLE_SIZE] = (i<TABLE_SIZE/slope)?-cos(i*M_PI/(TABLE_SIZE/slope)):INTERPOLATE((TABLE_SIZE-i)/(TABLE_SIZE-TABLE_SIZE/slope),-1,+1);
        data->FuncTable[i+2*TABLE_SIZE] = -((i<TABLE_SIZE/slope)?-cos(i*M_PI/(TABLE_SIZE/slope)):INTERPOLATE((TABLE_SIZE-i)/(TABLE_SIZE-TABLE_SIZE/slope),-1,+1));
        data->FuncTable[i+4*TABLE_SIZE] = sin(i*2*M_PI/8192+0.8*cos(i*6*M_PI/8192)+0.7*sin(i*10*M_PI/8192));
        data->FuncTable[i+5*TABLE_SIZE] = sin(M_PI*cos(i*2*M_PI/8192)+0.1*accum);
        data->FuncTable[i+6*TABLE_SIZE] = sin(i*2*M_PI/8192+0.3*cos(i*10*M_PI/8192)+0.4*cos(i*12*M_PI/8192));
    }

    for (i = 0; i < TABLE_SIZE / 2; i++) {
        double phs = (double)(i-TABLE_SIZE/4)/(TABLE_SIZE/4);
        data->FuncTable[i+3*TABLE_SIZE] = ((phs*phs-1));
        data->FuncTable[i+(int)(TABLE_SIZE*3.5)] = -((phs*phs-1));
	}

	plugin->data = data;
}

void phatman_destroy(struct linx_vertex_instance* plugin) {
	struct phatman* data = (struct phatman*)plugin->data;
	free(data);
}

struct linx_factory phatman_factory = {
	sizeof(struct linx_factory), "PhatMan", 0,
	(sizeof(phatman_pins) / sizeof(struct linx_pin)), phatman_pins,
	0, 0,
	phatman_create, phatman_destroy, phatman_process, phatman_describe_value
};

struct linx_factory* phatman_get_factory() {
	return &phatman_factory;
}

