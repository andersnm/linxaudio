// usage: 
//  var notes = KeySignature.getNotes(5, "7th", 0, "major"); // 7th chord at 5 degrees off c major scale => generates 4 notes for Amin7 chord
//  var notes = KeySignature.getNotes(2, "7th", 1, "minor"); // 7th chord at 2 degrees off c# minor scale => generates 4 notes for Fmaj7 chord

var KeySignature = {};

KeySignature.notes = [ "C", "C#", "D", "D#/Eb", "E", "F", "F#", "G", "G#/Ab", "A", "A#/Bb", "B" ];

KeySignature.scales = {
	"major" : [ 0, 2, 4, 5, 7, 9, 11 ],
	"minor" : [ 0, 2, 3, 5, 7, 8, 10 ],
};

KeySignature.chords = {
	"ONE" : [0],
	"tri" : [0,2,4],
	"7th" : [0,2,4,6],
	"sus" : [0,3,4],
	"dim" : [0,2,3],
	"aug" : [0,2,5],
};

KeySignature.getNotes = function(degree, type, keyNote, keyScaleName) {
	var scale = KeySignature.scales[keyScaleName]; // array of 7 notes which define a scale, entries are in "12-note-space"
	var chord = KeySignature.chords[type]; // array of note indexes into the scale, entries are in "7-note-space"
	var maxOctaves = 8; // assume we work on +/- maxOctaves
	var maxDegree = maxOctaves * scale.length; // used to avoid negative degree lookups
	var result = [];
	for (var i = 0; i < chord.length; i++) {
		var index = degree + chord[i];
		var indexOctave = Math.floor(index / scale.length);
		var note = (keyNote + scale[(index + maxDegree) % scale.length] + indexOctave * 12);
		result.push(note);
	}
	return result;
}
