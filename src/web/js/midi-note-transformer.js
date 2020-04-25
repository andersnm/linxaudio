function parseMidi(message) {
	return {
		status : message & 0xff,
		channel : message & 0x0f,
		command : (message & 0xf0) >> 4,
		data1 : (message >> 8) & 0xff,
		data2 : (message >> 16) & 0xff,
	};
}

function makeMidi(midiEvent) {
	return ((midiEvent.data2 & 0xff) << 16) | ((midiEvent.data1 & 0xff) << 8) | ((midiEvent.command & 0x0f) << 4) | (midiEvent.channel & 0x0f);	
}

function MidiNoteTransformer() {
	this.voices = [];
	this.type = 0;
}

MidiNoteTransformer.prototype.transformToMidi = function(value, key, scale) {
	// value is a note object with fields for degree, type, velocity
	// combine this with global key signature note + mode
	
	var result = [];
	// send stop notes for currently playing
	for (var i = 0; i < this.voices.length; i++) {
		var note = this.voices[i];
		var midiEvent = parseMidi(0);
		midiEvent.command = 9;
		midiEvent.data1 = note + 60 - 12;
		midiEvent.data2 = 0;
		result.push(makeMidi(midiEvent));
	}

	// then generate new notes
	if (value.velocity == 0) {
		// velocity=0 => just send note offs
		console.log("noteoffs!");
		return result;
	}

	var typeNames = ["ONE", "tri", "7th", "sus", "dim", "aug" ];
	var typeIndex = value.type != null ? value.type : this.type;
	var type = typeNames[typeIndex]; // pattern note object type is byte in range 0..5
	var degree = value.degree != null ? value.degree : 0; // pattern note object degree is byte in range -14..14 (3 octaves in 7-note-space)
	var notes = KeySignature.getNotes(degree, type, key, scale);//0, "minor");
	var velocity = value.velocity || 127;

	this.voices = notes;
	for (var i = 0; i < this.voices.length; i++) {
		var note = this.voices[i];
		var midiEvent = parseMidi(0);
		midiEvent.command = 9;
		midiEvent.data1 = note + 60 - 12; // 60 = midi middle c
		midiEvent.data2 = velocity;
		result.push(makeMidi(midiEvent));
	}
	
	this.type = typeIndex;
	
	return result;
	/*
	// decode single midi message
	var midiEvent = parseMidi(message);

	if (midiEvent.command == 0xb && midiEvent.data1 == 0x7b) {
		// all notes off - modules should handle this, but clear the voicemap
		this.voices.length = 0;
		return [ message ];
	} else if ((midiEvent.command == 9 && midiEvent.data2 == 0) || midiEvent.command == 8) {
		// noteoff - rewrite midi with actual note being released
		var lastNote = this.voices[repeatIndex];
		if (lastNote) {
			this.voices[repeatIndex] = 0;
			midiEvent.data1 = lastNote;
			return [ makeMidi(midiEvent) ];
		}
		return [];
	} else if (midiEvent.command == 9) {
		// return noteoff+note if tracker voice was already playing
		var lastNote = this.voices[repeatIndex];

		this.voices[repeatIndex] = midiEvent.data1;

		if (lastNote) {
			midiEvent.command = 9;
			midiEvent.data1 = lastNote;
			midiEvent.data2 = 0;			
			return [ makeMidi(midiEvent), message ];
		}
		return [ message ];
	} else {
		// anything other than note/noteoff - pass through
		return [ message ];
	}*/
}
