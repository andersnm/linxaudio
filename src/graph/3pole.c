

// called every pitchcounter%PITCHRESOLUTION
update_filter(float samplerate) {

	/// Cutoff scale computation ///
	if (Accstate == true) {
		Envmodinc += (0.125f);
	} else {
		Envmodinc += (0.125f * (1 - Decay));
	}
	Envmodphase = (1.0f / (1 + Envmodinc));
	Envmodphase = (Envmodphase * 0.965f + 0.035f) * Envmod + (Envmodphase * 0.05f + 0.1f) * (1.0f - Envmod);
	EnvmodphaseY = ((Envmodphase - EnvmodphaseZ) * 0.2f) + EnvmodphaseZ;	// Envmod
	EnvmodphaseZ = EnvmodphaseY;	// lowpass
	Cutfreq = EnvmodphaseY * (((acc[Accphase1] + acc[Accphase2] + acc[Accphase3]) * Acclevel) + 1.0f);
	Cutfreq = Cutfreq * Flpfold;
	if (Cutfreq > 0.87f) Cutfreq = 0.87f;
	Cutfreq = Cutfreq * (pMasterInfo->SamplesPerSec * 0.5f); //22050.0f;
	if (Accphase1 < 600) Accphase1 ++;
	if (Accphase2 < 600) Accphase2 ++;
	if (Accphase3 < 600) Accphase3 ++;
	//Oscfreq = oscphaseinc * 21.533203125f; //o
	Oscfreq = oscphaseinc * pMasterInfo->SamplesPerSec / 2048.0f;	//u
	if (Cutfreq < Oscfreq) Cutfreq = Oscfreq;
	Flpf = Cutfreq / (pMasterInfo->SamplesPerSec * 0.5f); //22050.0f;
	Flpf = fscale(Flpf);
	if (Flpf > 1) Flpf = 1.0f;
	Qdown = 1.0f - (float)pow(0.75f, Cutfreq / Oscfreq); // 0,8f def
	//Qdown = 1.0f;
	/// Q scale computation ///
	//cf = (Flpf * 1.012f) + 1;
	cf = (Flpf * 1.00f) + 1;
	//cf *= 0.5f;
	//Qlpfh = Qlpf * (float)( -0.9308 * pow(cf , 6) + 5.398 * pow(cf , 5) - 11.753 * pow(cf , 4) + 10.655 * pow (cf , 3) - 0.416 * pow (cf , 2) - 7.0114 * cf + 5.9039);
	Qlpfh = 5.9039f - 7.0114f * cf;
	cf *= (Flpf + 1);
	Qlpfh = Qlpfh - 0.416f * cf;
	cf *= (Flpf + 1);
	Qlpfh = Qlpfh + 10.655f * cf;
	cf *= (Flpf + 1);
	Qlpfh = Qlpfh	- 11.753f * cf;
	cf *= (Flpf + 1);
	Qlpfh = Qlpfh + 5.398f * cf;
	cf *= (Flpf + 1);
	Qlpfh = Qlpfh - 0.9308f * cf;
	Qlpfh = Qlpfh * Qlpf;
	Qlpfh *= Qdown;
	///////////////////////////
	pitchcounter = 0;

}


float process_sample(float in) {
	// 3p Lowpass Resonant VCF ///
	float out = in * (Qlpfh + 1);
	//out = out * amlv; //amlv == gain
	out = out - Yc * Qlpfh;
	Flpfh = (Flpf + 1) * 0.5f;
	Xaz = Xa;
	Xa = out;
	Yaz = Ya;
	Ya = ((Xa + Xaz) * Flpfh) - (Flpf * Yaz);
	out = Ya;
	Xbz = Xb;
	Xb = out;
	Ybz = Yb;
	Yb = ((Xb + Xbz) * Flpfh) - (Flpf * Ybz);
	out = Yb;
	Xcz = Xc;
	Xc = out;
	Ycz = Yc;
	Yc = ((Xc + Xcz) * Flpfh) - (Flpf * Ycz);
	out = Yc;

	// Allpass shifter ///
	hFh = -0.998f; // -0.994f, 996
	hXa = out;
	hYa = hXa * hFh + hYaz;
	hYaz = hXa - hFh * hYa;
	out = hYa;
	// Clipper def -2, 2 ///
	if (out < -14.0f * 8192.0f) out = -14.0f * 8192.0f;
	if (out > 14.0f * 8192.0f) out = 14.0f * 8192.0f;
	///////////////////
	hXb = out;
	hYb = hXb * hFh + hYbz;
	hYbz = hXb - hFh * hYb;
	out = hYb;
	return out;
}
