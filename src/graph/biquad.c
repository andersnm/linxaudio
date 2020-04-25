/*
biquad.c - C port of Krzysztof Foltman's LGPL CBiquad filters

TODO: this is largely untested - the modules pin ranges must be fixed
before any cereal use
*/
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mathext.h"
#include "linxaudio.h"

#define TWOPI_F (2.0f*(float)M_PI)

struct biquad {
	float a1, a2, b0, b1, b2;
	float x1, x2, y1, y2;
};

void biquad_init(struct biquad* b) {
	b->x1 = 0.0f;
	b->y1 = 0.0f;
	b->x2 = 0.0f;
	b->y2 = 0.0f;
}

inline float biquad_process_sample(struct biquad* b, float dSmp) { 
	float dOut = b->b0 * dSmp + b->b1 * b->x1 + b->b2 * b->x2 - b->a1 * b->y1 - b->a2 * b->y2;
	b->y2 = b->y1;
	b->y1 = dOut;
	b->x2 = b->x1;
	b->x1 = dSmp;
	return dOut;
}

// Robert Bristow-Johnson, robert@audioheads.com
void biquad_rbjLPF(struct biquad* b, float fc, float Q, float esr, float gain) {
	float omega = 2.0f * (float)M_PI * fc / esr;
	float sn = sinf(omega);
	float cs = cosf(omega);
	float alpha = (sn / (2.0f * Q));
	float inv = (1.0f / (1.0f + alpha));
	b->b0 = (gain * inv * (1.0f - cs) / 2.0f);
	b->b1 = (gain * inv * (1.0f - cs));
	b->b2 = (gain * inv * (1.0f - cs) / 2.0f);
	b->a1 = (-2.0f * cs * inv);
	b->a2 = ((1.0f - alpha) * inv);
}

void biquad_rbjHPF(struct biquad* b, float fc, float Q, float esr, float gain) {
	float omega = 2.0f * (float)M_PI * fc / esr;
	float sn = sinf(omega);
	float cs = cosf(omega);
	float alpha = (sn / (2.0f * Q));
	float inv = (1.0f / (1.0f + alpha));
	b->b0 = (gain *  inv * (1.0f + cs) / 2.0f);
	b->b1 = (gain * -inv * (1.0f + cs));
	b->b2 = (gain *  inv * (1.0f + cs) / 2.0f);
	b->a1 = (-2.0f * cs * inv);
	b->a2 = ((1.0f - alpha) * inv);
}

void biquad_rbjBPF(struct biquad* b, float fc, float Q, float esr, float gain) {
	float omega = 2.0f * (float)M_PI * fc / esr;
	float sn = sinf(omega);
	float cs = cosf(omega);
	float alpha = (sn / (2.0f * Q));
	float inv = (1.0f / (1.0f + alpha));
	b->b0 = (gain * inv * alpha);
	b->b1 = 0.0f;
	b->b2 = (-gain * inv * alpha);
	b->a1 = (-2.0f * cs * inv);
	b->a2 = ((1.0f - alpha) * inv);
}

void biquad_rbjBRF(struct biquad* b, float fc, float Q, float esr, float gain) {
	float omega = 2.0f * (float)M_PI * fc / esr;
	float sn = sinf(omega);
	float cs = cosf(omega);
	float alpha = (sn / (2.0f * Q));
	float inv = (1.0f / (1.0f + alpha));
	b->b0 = (gain * inv);
	b->b1 = (-gain * inv * 2.0f * cs);
	b->b2 = (gain * inv);
	b->a1 = (-2.0f * cs * inv);
	b->a2 = ((1.0f - alpha) * inv);
}

/*
void biquad_rbjBPF2(struct biquad* b, float fc, float bw, float esr) {
	float omega = (2.0f * M_PI * fc / esr);
	float sn = sin(omega);
	float cs = cos(omega);
	float alpha = (sn / sinh(log(2.0f) / 2.0f * bw * omega / sn));
	float inv = (1.0f / (1.0f + alpha));
	b->b0 = (inv * alpha);
	b->b1 = 0.0f;
	b->b2 = (-inv * alpha);
	b->a1 = (-2.0f * cs * inv);
	b->a2 = ((1.0f - alpha)*inv);
}

void biquad_rbjBRF2(struct biquad* b, float fc, float bw, float esr) {
	float omega = (2.0f * M_PI * fc / esr);
	float sn = sin(omega);
	float cs = cos(omega);
	float alpha = (sn / sinh(log(2.0f) / 2.0f * bw * omega / sn));
	float inv = (1.0f / (1.0f + alpha));
	b->b0 = (inv);
	b->b1 = (-inv * 2.0f * cs);
	b->b2 = (inv);
	b->a1 = (-2.0f * cs * inv);
	b->a2 = ((1.0f - alpha) * inv);
}
*/

// Zoelzer's Parmentric Equalizer Filters - rodem z Csound'a
void biquad_set_lowshelf(struct biquad* b, float fc, float q, float v, float esr, float gain) {
	float sq = sqrtf(2.0f * v);
	float omega = TWOPI_F * fc / esr;
	float k =  tanf(omega * 0.5f);
	float kk = k * k;
	float vkk = v * kk;
	float oda0 =  1.0f / (1.0f + k / q +kk);
	b->b0 = oda0 * (1.0f + sq * k + vkk);
	b->b1 = oda0 * (2.0f * (vkk - 1.0f));
	b->b2 = oda0 * (1.0f - sq * k + vkk);
	b->a1 = oda0 * (2.0f * (kk - 1.0f));
	b->a2 = oda0 * (1.0f - k / q + kk);
}

void biquad_set_highshelf(struct biquad* b, float fc, float q, float v, float esr, float gain) {
	float sq = sqrtf(2.0f * v);
	float omega = TWOPI_F * fc / esr;
	float k =  tanf((M_PI - omega) * 0.5f);
	float kk = k * k;
	float vkk = v * kk;
	float oda0 = 1.0f / ( 1.0f + k / q + kk);
	b->b0 = oda0 * ( 1.0f + sq * k + vkk) * gain;
	b->b1 = oda0 * (-2.0f * (vkk - 1.0f)) * gain;
	b->b2 = oda0 * ( 1.0f - sq * k + vkk) * gain;
	b->a1 = oda0 * (-2.0f * (kk - 1.0f));
	b->a2 = oda0 * ( 1.0f - k / q + kk);
}

void biquad_set_parametric_eq(struct biquad* b, float fc, float q, float v, float esr, float gain) {
	float omega = (TWOPI_F * fc / esr);
	float k =  tanf(omega * 0.5f);
	float kk = k * k;
	float vk = (v * k);
	float vkdq = (vk / q);
	float oda0 =  (1.0f / (1.0f + k / q +kk));
	b->b0 = gain * oda0 * (1.0f + vkdq + kk);
	b->b1 = gain * oda0 * (2.0f * (kk - 1.0f));
	b->b2 = gain * oda0 * (1.0f - vkdq + kk);
	b->a1 = oda0 * (2.0f * (kk - 1.0f));
	b->a2 = oda0 * (1.0f - k/q + kk);
}

/*
	biquad_filter module
*/

struct biquad_filter {
	struct biquad b;
	int type;
	float cutoff, q;
	float samplerate;
};

void biquad_filter_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct linx_buffer* in_buffer = &pin_buffers[0];
	struct linx_buffer* out_buffer = &pin_buffers[1];
	struct biquad_filter* data = (struct biquad_filter*)self->data;
	int changed = 0;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			data->type = v->intvalue;
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->cutoff = v->floatvalue;
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			data->q = v->floatvalue;
			changed = 1;
		}
	}
	if (changed) {
		switch (data->type) {
			case 0:
				biquad_rbjLPF(&data->b, data->cutoff, data->q, data->samplerate, 1.0f);
				break;
			case 1:
				biquad_rbjHPF(&data->b, data->cutoff, data->q, data->samplerate, 1.0f);
				break;
			case 2:
				biquad_rbjBPF(&data->b, data->cutoff, data->q, data->samplerate, 1.0f);
				break;
			case 3:
				biquad_rbjBRF(&data->b, data->cutoff, data->q, data->samplerate, 1.0f);
				break;
			default:
				assert(0);
				break;
		}
	}

	if (in_buffer->write_count > 0) {
		float* in_ptr = in_buffer->float_buffer;
		float* out_ptr = out_buffer->float_buffer;

		while (sample_count--) {
			*out_ptr = biquad_process_sample(&data->b, *in_ptr);
			out_ptr++;
			in_ptr++;
		}

		out_buffer->write_count = 1;
	}
}

const char* biquad_filter_type_names[] = {
	"LPF", "HPF", "BPF", "BRF"
};

void biquad_filter_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 2:
			// type
			snprintf(result_name, result_name_size, "%s", biquad_filter_type_names[(int)value]);
			break;
		case 3:
			// cutoff
			snprintf(result_name, result_name_size, "%0.0f Hz", value);
			break;
	}
}

void biquad_filter_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct biquad_filter* data = (struct biquad_filter*)malloc(sizeof(struct biquad_filter));
	memset(data, 0, sizeof(struct biquad_filter));
	data->samplerate = (float)samplerate;
	plugin->data = data;
}

void biquad_filter_destroy(struct linx_vertex_instance* plugin) {
	struct biquad_filter* data = (struct biquad_filter*)plugin->data;
	free(data);
}


/*
	biquad_eq module
*/

struct biquad_eq {
	struct biquad b;
	int type;
	float cutoff, q, v;
	float samplerate;
};

void biquad_eq_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct linx_buffer* in_buffer = &pin_buffers[0];
	struct linx_buffer* out_buffer = &pin_buffers[1];
	struct biquad_eq* data = (struct biquad_eq*)self->data;
	int changed = 0;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			data->type = v->intvalue;
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->cutoff = v->floatvalue;
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			data->q = v->floatvalue;
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 5) {
			data->v = v->floatvalue;
			changed = 1;
		}
	}
	if (changed) {
		switch (data->type) {
			case 0:
				biquad_set_lowshelf(&data->b, data->cutoff, data->q, data->v, data->samplerate, 1.0f);
				break;
			case 1:
				biquad_set_highshelf(&data->b, data->cutoff, data->q, data->v, data->samplerate, 1.0f);
				break;
			case 2:
				biquad_set_parametric_eq(&data->b, data->cutoff, data->q, data->v, data->samplerate, 1.0f);
				break;
			default:
				assert(0);
				break;
		}
	}

	if (in_buffer->write_count > 0) {
		float* out_ptr = out_buffer->float_buffer;
		float* in_ptr = in_buffer->float_buffer;
		while (sample_count--) {
			*out_ptr = biquad_process_sample(&data->b, *in_ptr);
			out_ptr++;
			in_ptr++;
		}

		out_buffer->write_count = 1;
	}
}

char* biquad_eq_type_names[] = {
	"Lowshelf", 
	"Highshelf", 
	"ParamEQ"
};

void biquad_eq_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 2:
			snprintf(result_name, result_name_size, "%s", biquad_eq_type_names[(int)value]);
			break;
		case 3:
			// cutoff
			snprintf(result_name, result_name_size, "%0.0f Hz", value);
			break;
	}
}

void biquad_eq_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct biquad_eq* data = (struct biquad_eq*)malloc(sizeof(struct biquad_eq));
	memset(data, 0, sizeof(struct biquad_eq));
	data->samplerate = (float)samplerate;
	plugin->data = data;
}

void biquad_eq_destroy(struct linx_vertex_instance* plugin) {
	struct biquad_eq* data = (struct biquad_eq*)plugin->data;
	free(data);
}

/*
	biquad_trifilter module
*/

struct biquad_trifilter {
	struct biquad b1;
	struct biquad b2;
	struct biquad b3;
	int type;
	float cutoff, q;
	float thevfactor;
	float samplerate;
};

float inline clamp_float(float f, float minvalue, float maxvalue) {
	return fmaxf(fminf(f, maxvalue), minvalue);
}

void biquad_trifilter_calc_coeffs1(struct biquad_trifilter* data) { // 6L Multipeak 
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 20000.0f, data->thevfactor);
	float fQ = 0.707f + 7.0f * data->q * scalereso;
	biquad_rbjLPF(&data->b1, cf / 3.0f, fQ, data->samplerate, sqrtf(0.707f) / sqrtf(fQ));
	biquad_rbjLPF(&data->b2, 2.0f * cf / 3.0f, fQ / 2, data->samplerate, 1.0);
	biquad_rbjLPF(&data->b3, cf, fQ / 3.0f, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs2(struct biquad_trifilter* data) { // 6L Separated
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 16000.0f);
	float scalereso = powf(cf / 22000.0f, data->thevfactor);
	float fQ = 1.50f + 10.6f * data->q * scalereso;
	float sep = 0.05f + 0.6f * data->q;
	biquad_rbjLPF(&data->b1, cf, fQ, data->samplerate, 0.3f / powf(fQ / 2.50f, 0.05f));
	biquad_rbjLPF(&data->b2, cf * (1.0f - sep), fQ, data->samplerate, 1.0f);
	biquad_rbjLPF(&data->b2, fminf(cf * (1.0f + sep), 21000.0f), fQ, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs3(struct biquad_trifilter* data) { // 6L HiSquelch
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 20000.0f, data->thevfactor);
	float fQ = 0.71f + 10.0f * data->q * scalereso;
	biquad_rbjLPF(&data->b1, cf, fQ, data->samplerate, 0.6f / powf(fmaxf(fQ, 1.0f), 1.7f));
	biquad_rbjLPF(&data->b2, cf, fQ, data->samplerate, 1.0f);
	biquad_rbjLPF(&data->b3, cf, fQ, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs4(struct biquad_trifilter* data) { // 4L Skull D
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 21000.0f, data->thevfactor);
	float fQ = 1.0f + 10.0f * data->q * scalereso;
	biquad_rbjLPF(&data->b1, cf, 0.707f, data->samplerate, 0.5f);
	biquad_rbjLPF(&data->b2, cf, 0.707f, data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cf, fQ * 4.0f, fQ * 2.0f, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs5(struct biquad_trifilter* data) { // 4L TwinPeaks
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 20000.0f, data->thevfactor);
	float fQ = 0.71f + 5.0f * data->q * scalereso;
	biquad_rbjLPF(&data->b1, cf, fQ, data->samplerate, 0.3f / fmaxf(sqrtf(fQ) * fQ, 1.0f));
	biquad_rbjLPF(&data->b2, cf, fQ, data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cf / 2.0f, 3.0f * (fQ - 0.7f) + 1.0f, 8.0f * (fQ - 0.7f) + 1.0f, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs6(struct biquad_trifilter* data) { // 4L Killah
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 20000.0f, data->thevfactor);
	float fQ = 0.71f + 5.0f * data->q * scalereso;
	biquad_rbjLPF(&data->b1, cf / 1.41f, fQ, data->samplerate, 0.6f / fmaxf(sqrtf(fQ) * fQ, 1.0f));
	biquad_rbjLPF(&data->b2, fminf(cf * 1.41f, 22000.0f), fQ, data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cf, 16.0f / fQ, fQ * 4.0f, data->samplerate, 1.0);
}

void biquad_trifilter_calc_coeffs7(struct biquad_trifilter* data) { // 4L Phlatt
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 20000.0f, data->thevfactor);
	float fQ = 0.71f + 5.0f * data->q * scalereso;
	biquad_rbjLPF(&data->b1, cf, fQ, data->samplerate, 0.8f / fmaxf(fQ, 1.0f));
	biquad_rbjLPF(&data->b2, cf, fQ, data->samplerate, 1.0f);
	biquad_rbjBRF(&data->b3, cf, fQ, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs8(struct biquad_trifilter* data) { // 2L phlatt
	float cf = data->cutoff; // clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float fQ = 1.0f + 4.0f * (1.0f - data->q);
	biquad_rbjLPF(&data->b1, cf, 1.007f, data->samplerate, 0.8f / fmaxf(sqrtf(fQ), 1.0f));
	biquad_rbjBRF(&data->b2, cf * 0.707f, fQ / 2.0f, data->samplerate, 1.0f);
	biquad_rbjBRF(&data->b3, cf, fQ / 2.0f, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs9(struct biquad_trifilter* data) { // 2L FrontFlt
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 22000.0f, data->thevfactor);
	float fQ = 0.71f + 6.0f * data->q * scalereso;
	biquad_rbjLPF(&data->b1, cf, 2.0f * fQ, data->samplerate, 0.3f / fmaxf(sqrtf(fQ), 1.0f));
	biquad_set_parametric_eq(&data->b2, cf / 2.0f, 3.0f * (fQ - 0.7f) + 1.0f, 3.0f * (fQ - 0.7f) + 1.0f, data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cf / 4.0f, 3.0f * (fQ - 0.7f) + 1.0f, 3.0f * (fQ - 0.7f) + 1.0f, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs10(struct biquad_trifilter* data) { // 2L LaserOne
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 20000.0f, data->thevfactor);
	float fQ = 0.71f + 6.0f * data->q * scalereso;
	biquad_rbjLPF(&data->b1, cf, 2.0f * fQ, data->samplerate, (0.15f / fmaxf(sqrtf(fQ), 1.0f)));
	biquad_set_parametric_eq(&data->b2, cf * 3.0f / 4.0f, 2.0f * (fQ - 0.7f) + 1.0f, 3.0f * (fQ - 0.7f) + 1.0f, data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cf / 2.0f, 2.0f * (fQ - 0.7f) + 1.0f, 3.0f * (fQ - 0.7f) + 1.0f, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs11(struct biquad_trifilter* data) { // 2L FMish
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 20000.0f, data->thevfactor);
	float fQ = 0.71f + 6.0f * scalereso;
	float sc1 = powf(fminf(0.89f, 0.33f + 0.2f * data->cutoff), 1.0f - data->q + 0.5f);
	float sc2 = powf(fminf(0.9f, 0.14f + 0.1f * data->cutoff), 1.0f - data->q + 0.5f);
	biquad_rbjLPF(&data->b1, cf, 2.0f * fQ, data->samplerate, 0.2f / fmaxf(sqrtf(fQ), 1.0f));
	biquad_set_parametric_eq(&data->b2, cf * sc1, 2.0f * (fQ - 0.7f) + 1.0f, 3.0f * (fQ - 0.7f) + 1.0f, data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cf * sc2, 2.0f * (fQ - 0.7f) + 1.0f, 3.0f * (fQ - 0.7f) + 1.0f, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs12(struct biquad_trifilter* data) { // Notchez
	float cf = 20000.0f - data->cutoff; // clamp_float(132.0f * powf(64.0f, (240.0f - data->cutoff) / 240.0), 33.0f, 20000.0f);
	float q = 0.1f + data->q * 0.6f;
	float spacing = powf(1.3f + 3.0f * (1.0f - data->q), 1.0f - cf / 20000.0f);
	biquad_rbjBRF(&data->b1, cf, q, data->samplerate, 1.0f);
	biquad_rbjBRF(&data->b2, cf / spacing, q, data->samplerate, 1.0f);
	biquad_rbjBRF(&data->b3, fminf(21000.0f, cf * spacing), q, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs13(struct biquad_trifilter* data) { // 6H Relaxed
	float cf = data->cutoff; //clamp_float(66.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float q = 0.71f + data->q * 2.6f;
	float spacing = powf(1.3f + 3.0f * (1.0f - data->q), 1.0f - cf / 20000.0f);
	biquad_rbjHPF(&data->b1, cf, q, data->samplerate, 0.71f / powf(q, 0.7f));
	biquad_rbjHPF(&data->b2, cf / spacing, q, data->samplerate, 1.0f);
	biquad_rbjHPF(&data->b3, fminf(21000.0f, cf * spacing), q, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs14(struct biquad_trifilter* data) { // 6B Plain
	float cf = data->cutoff; // clamp_float(66.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float scalereso = powf(cf / 20000.0f, data->thevfactor);
	float q = 0.1f + scalereso * data->q * 2.6f;
	biquad_rbjBPF(&data->b1, cf, q, data->samplerate, powf(q, 0.7f) / 1.7f);
	biquad_rbjBPF(&data->b2, cf * 0.9f, q, data->samplerate, 1.0f);
	biquad_rbjBPF(&data->b3, fminf(21000.0f, cf * 1.01f), q, data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs15(struct biquad_trifilter* data) { // 6X BatGuy
	float cf = data->cutoff; //clamp_float(132.0f * powf(64.0f, data->cutoff / 240.0f), 33.0f, 20000.0f);
	float q = 2.1f + data->q * 9.6f;
	biquad_set_parametric_eq(&data->b1, (cf / 4.0f), 1.0f, q, data->samplerate, (0.25f / sqrtf(q)));
	biquad_set_parametric_eq(&data->b2, (cf / 2.0f), 2.0f, (1.0f / q), data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cf, 1.0f, q, data->samplerate, 1.0f);
}

float threesel(float sel, float a, float b, float c) {
	//#define THREESEL(sel,a,b,c) ((sel)<120.0f)?((a)+((b)-(a))*(sel)/120.0f):((b)+((c)-(b))*((sel)-120.0f)/120.0f)
	// 0..240 = 33..20000
	const float half = (20000.0f - 33.0f) / 2.0f; //120.0f;
	return ((sel)<half)?((a)+((b)-(a))*(sel)/half):((b)+((c)-(b))*((sel)-half)/half);
}

void biquad_trifilter_calc_coeffs16(struct biquad_trifilter* data) { // 6X Vocal1
	float q = clamp_float(2.1f + data->q * 32.6f, 0.0f, 1.0f);
	float cutoff1 = threesel(data->cutoff, 270.0f, 800.0f, 400.0f);
	float cutoff2 = threesel(data->cutoff, 2140.0f,1150.0f, 800.0f);
	biquad_set_parametric_eq(&data->b1, cutoff1, 2.5f, q, data->samplerate, (1.0f / q));
	biquad_rbjLPF(&data->b2, cutoff2 * 1.2f, sqrtf(q), data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cutoff2, 2.5f, sqrtf(q), data->samplerate, 1.0f);
}

void biquad_trifilter_calc_coeffs17(struct biquad_trifilter* data) { // 6X Vocal2
	float cf = data->cutoff; //clamp_float(data->cutoff, 0.0f, 240.0f);
	float q = 2.1f + data->q *32.6f;
	float cutoff1 = threesel(cf, 650.0f, 400.0f, 270.0f);
	float cutoff2 = threesel(cf, 1080.0f, 1700.0f, 2140.0f);
	biquad_set_parametric_eq(&data->b1, cutoff1, 2.5f, q, data->samplerate, (1.0f / q));
	biquad_rbjLPF(&data->b2, cutoff2 * 1.2f, sqrtf(q), data->samplerate, 1.0f);
	biquad_set_parametric_eq(&data->b3, cutoff2, 2.5f, sqrtf(q), data->samplerate, 1.0f);
}

void biquad_trifilter_process(struct linx_vertex_instance* self, struct linx_value_array* in_values, struct linx_buffer* pin_buffers, struct linx_buffer* propagated_pin_buffers, struct linx_value_array* out_values, int sample_count) {
	int i;
	struct linx_buffer* in_buffer = &pin_buffers[0];
	struct linx_buffer* out_buffer = &pin_buffers[1];
	struct biquad_trifilter* data = (struct biquad_trifilter*)self->data;
	int changed = 0;

	for (i = 0; i < in_values->length; i++) {
		struct linx_value* v = &in_values->items[i];
		if (v->pin_group == linx_pin_group_module && v->pin_index == 2) {
			data->type = v->intvalue;
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 3) {
			data->cutoff = v->floatvalue;
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			data->q = v->floatvalue;
			changed = 1;
		} else if (v->pin_group == linx_pin_group_module && v->pin_index == 4) {
			data->thevfactor = v->floatvalue;
			changed = 1;
		}
	}
	if (changed) {
		switch (data->type) {
			case  0: biquad_trifilter_calc_coeffs1(data); break;
			case  1: biquad_trifilter_calc_coeffs2(data); break;
			case  2: biquad_trifilter_calc_coeffs3(data); break;
			case  3: biquad_trifilter_calc_coeffs4(data); break;
			case  4: biquad_trifilter_calc_coeffs5(data); break;
			case  5: biquad_trifilter_calc_coeffs6(data); break;
			case  6: biquad_trifilter_calc_coeffs7(data); break;
			case  7: biquad_trifilter_calc_coeffs8(data); break;
			case  8: biquad_trifilter_calc_coeffs9(data); break;
			case  9: biquad_trifilter_calc_coeffs10(data); break;
			case 10: biquad_trifilter_calc_coeffs11(data); break;
			case 11: biquad_trifilter_calc_coeffs12(data); break;
			case 12: biquad_trifilter_calc_coeffs13(data); break;
			case 13: biquad_trifilter_calc_coeffs14(data); break;
			case 14: biquad_trifilter_calc_coeffs15(data); break;
			case 15: biquad_trifilter_calc_coeffs16(data); break;
			case 16: biquad_trifilter_calc_coeffs17(data); break;
			default:
				assert(0);
				break;
		}
	}

	if (in_buffer->write_count > 0) {
		float* out_ptr = out_buffer->float_buffer;
		float* in_ptr = in_buffer->float_buffer;
		while (sample_count--) {
			float v = biquad_process_sample(&data->b1, *in_ptr);
			v = biquad_process_sample(&data->b2, v);
			v = biquad_process_sample(&data->b3, v);
			*out_ptr = v;
			out_ptr++;
			in_ptr++;
		}
		out_buffer->write_count = 1;
	}
}

char* biquad_trifilter_type_names[] = {
	"6L Multipeak", 
	"6L Separated", 
	"6L HiSquelch",
	"4L Skull D",
	"4L TwinPeaks",
	"4L Killah",
	"4L Phlatt",
	"2L Phlatt",
	"2L FrontFlt",
	"2L LaserOne",
	"2L FMish",
	"Notchez",
	"6H Relaxed",
	"6B Plain",
	"6X BatGuy",
	"6X Vocal1",
	"6X Vocal2",
	"No Filter"
};

void biquad_trifilter_describe_value(struct linx_vertex_instance* self, int pin_index, float value, char* result_name, int result_name_size) {
	switch (pin_index) {
		case 2:
			snprintf(result_name, result_name_size, "%s", biquad_trifilter_type_names[(int)value]);
			break;
		case 3:
			// cutoff
			snprintf(result_name, result_name_size, "%0.0f Hz", value);
			break;
	}
}

void biquad_trifilter_create(struct linx_vertex_instance* plugin, int samplerate) {
	struct biquad_trifilter* data = (struct biquad_trifilter*)malloc(sizeof(struct biquad_trifilter));
	memset(data, 0, sizeof(struct biquad_trifilter));
	data->samplerate = (float)samplerate;
	plugin->data = data;
}

void biquad_trifilter_destroy(struct linx_vertex_instance* plugin) {
	struct biquad_trifilter* data = (struct biquad_trifilter*)plugin->data;
	free(data);
}

/*
	Module factories
*/

struct linx_pin biquad_filter_pins[] = {
	{ "In", linx_pin_type_in_buffer_float },
	{ "Out", linx_pin_type_out_buffer_float },
	{ "Type", linx_pin_type_in_scalar_int, 0.0f, 3.0f, 0.0f, 1.0f },
	{ "Cutoff", linx_pin_type_in_scalar_float, 33.0f, 20000.0f, 10000.0f, 1.0f },
	{ "Q", linx_pin_type_in_scalar_float, 0.01f, 1.0f, 1.0f, 0.01f  }
};

struct linx_pin biquad_eq_pins[] = {
	{ "In", linx_pin_type_in_buffer_float },
	{ "Out", linx_pin_type_out_buffer_float },
	{ "Type", linx_pin_type_in_scalar_int, 0.0f, 2.0f, 2.0f, 1.0f },
	{ "Cutoff", linx_pin_type_in_scalar_float, 33.0f, 20000.0f, 10000.0f, 1.0f },
	{ "Q", linx_pin_type_in_scalar_float, 0.01f, 1.0f,  0.01f, 0.01f  },
	{ "V", linx_pin_type_in_scalar_float, 0.0f, 2.0f,  0.0f, 0.01f  }
};

struct linx_pin biquad_trifilter_pins[] = {
	{ "In", linx_pin_type_in_buffer_float },
	{ "Out", linx_pin_type_out_buffer_float },
	{ "Type", linx_pin_type_in_scalar_int, 0.0f, 16.0f, 0.0f, 1.0f },
	{ "Cutoff", linx_pin_type_in_scalar_float, 33.0f, 20000.0f, 10000.0f, 1.0f },
	{ "Resonance", linx_pin_type_in_scalar_float, 0.0f, 1.0f,  0.0f, 0.01f  },
	{ "Thevfactor", linx_pin_type_in_scalar_float, 0.0f, 1.0f,  0.4f, 0.01f  }
};

struct linx_factory biquad_filter_factory = {
	sizeof(struct linx_factory), "BiquadFilter", 0,
	(sizeof(biquad_filter_pins) / sizeof(struct linx_pin)), biquad_filter_pins,
	0, 0,
	biquad_filter_create, biquad_filter_destroy, biquad_filter_process, biquad_filter_describe_value
};

struct linx_factory biquad_eq_factory = {
	sizeof(struct linx_factory), "BiquadEq", 0,
	(sizeof(biquad_eq_pins) / sizeof(struct linx_pin)), biquad_eq_pins,
	0, 0,
	biquad_eq_create, biquad_eq_destroy, biquad_eq_process, biquad_eq_describe_value
};

struct linx_factory biquad_trifilter_factory = {
	sizeof(struct linx_factory), "BiquadTrifilter", 0,
	(sizeof(biquad_trifilter_pins) / sizeof(struct linx_pin)), biquad_trifilter_pins,
	0, 0,
	biquad_trifilter_create, biquad_trifilter_destroy, biquad_trifilter_process, biquad_trifilter_describe_value
};

struct linx_factory* biquad_trifilter_get_factory() {
	return &biquad_trifilter_factory;
}

struct linx_factory* biquad_eq_get_factory() {
	return &biquad_eq_factory;
}

struct linx_factory* biquad_filter_get_factory() {
	return &biquad_filter_factory;
}
