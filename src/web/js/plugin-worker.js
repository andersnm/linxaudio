var cache = Date.now().toString();
Module = {};
Module.TOTAL_MEMORY = 1024*1024*64;

importScripts(
	"linx.js?"+cache, 
	"linx-util.js?"+cache, 
	"util/minivents.js?"+cache, 
	"plugin-project.js?"+cache, 
	"plugin-host.js?"+cache, 
	"plugin-compiler.js?"+cache, 
	"midi-note-transformer.js?"+cache, 
	"key-signature.js?"+cache
);

var factories = createFactories();
var project = new LinxProject(factories);
var graphPtr = null;
var instancePtr = null;
var host = null;
var playing = false;
var inputBuffer = null;
var outputBuffers = [];
var BUFFER_SIZE = 0;

var messages = [];
var samplePosition = 0;
var currentTickPosition = 0;
var currentTick = 0;
var samplesPerTick = 0;
var sampleRate = 44100;


onmessage = function(e) {
	switch (e.data.command) {
		case "playFromStart":
			currentTickPosition = 0;
			currentTick = 0;
			playing = true;
			break;
		case "play":
			playing = true;
			break;
		case "stop":
			host.stopNotes(messages, samplePosition);
			playing = false;
			break;
		case "setPosition":
			currentTickPosition = 0;
			currentTick = e.data.value;
			break;
		case "setBpm":
			project.bpm = e.data.value;
			updateTempo();
			break;
		case "setTpb":
			project.tpb = e.data.value;
			updateTempo();
			break;
		case "setKey":
			project.key = e.data.value;
			break;
		case "setScale":
			project.scale = e.data.value;
			break;
		case "compileProject":
			project.clear();
			if (instancePtr != null) {
				Module._linx_graph_instance_destroy(instancePtr);
				Module._linx_graph_definition_destroy(graphPtr);
				instancePtr = null;
				graphPtr = null;
				host = null;
			}
			if (project.parseJsonProject(e.data.project)) {
				graphPtr = LinxCompiler.compile(project.graph);
				host = new LinxHost(graphPtr);
				instancePtr = Module._linx_graph_definition_create_instance(graphPtr, sampleRate);
				updateTempo();
				createBufferQueues();
			} else {
				console.log("worker compile json parsing failed");
			}
			break;
		case "setBufferSize":
			BUFFER_SIZE = e.data.value;
			createBufferQueues();
			break;
		case "setSampleRate":
			sampleRate = e.data.value;
			break;
		case "describeValue":
			var description = describeValue(e.data.graphId, e.data.vertexId, e.data.pinName, e.data.value);
			postMessage({ command : "describeValueResponse", value : description });
			break;
		case "processNote":
			processNote(e.data.graphId, e.data.vertex, e.data.pinName, e.data.note, e.data.velocity);
			break;
		case "parameterChange":
			parameterChange(e.data.graphId, e.data.vertex, e.data.pinName, e.data.value);
			break;
		case "getEdgeDigest":
			var buffer = getEdgeDigest(e.data.graphId);
			var transferObjects = buffer ? [buffer.buffer] : [];
			postMessage({
				command : "getEdgeDigestResponse", 
				buffer : buffer
			}, transferObjects);
			break;
		case "processAudio":
			var buffer = processAudio(e.data.numSamples);
			var transferObjects = [];
			for (var i = 0; i < buffer ? buffer.channels.length : 0; i++) {
				transferObjects[i] = buffer.channels[i].buffer;
			}
			postMessage({
				command : "processAudioResponse",
				buffer : buffer
			}, transferObjects);
			break;
		case "returnAudioBuffer":
			// a pair of buffers have been transfered to us!
			if (e.data.buffer.channels.length == host.outAudioPinIndexes.length) {
				outputBuffers.push(e.data.buffer);
			} else {
				console.log("INFO: discarding buffers returned to worker because of channel count mismatch");
			}
			break;
		case "getFactories":
			postMessage({
				command:"getFactoriesResponse", 
				factories : factories
			});
			break;
		default:
			console.log("worker got unhandled message", e);
			break;
	}
}

function AudioBuffer(channelCount, sampleCount) {
	this.channels = [];
	for (var i = 0; i < channelCount; i++) {
		this.channels.push(new Float32Array(sampleCount));
	}
}

AudioBuffer.prototype.getChannelData = function(index) {
	return this.channels[index];
}

function updateTempo() {
	var tpm = project.bpm * project.tpb;
	samplesPerTick = (60.0 * sampleRate) / tpm;
}

function createBuffers(bufferCount, channelCount, sampleCount) {
	var result = [];
	for (var i = 0; i < bufferCount; i++) {
		var buffer = new AudioBuffer(channelCount, sampleCount);
		result.push(buffer);
	}
	return result;
}

function createBufferQueues() {
	// call after recompile and buffer length changes etc
	// INSTEAD: call only on init, assume we use stereo buffers only!
	if (host == null) {
		return ;
	}
	inputBuffer = new AudioBuffer(host.inAudioPinIndexes.length, BUFFER_SIZE);
	outputBuffers = createBuffers(8, host.outAudioPinIndexes.length, BUFFER_SIZE);
}


function processAudio(numSamples) {

	if (host == null) {
		return null;
	}

	if (playing) {
		processSequencer(numSamples);
	}

	if (outputBuffers.length == 0) {
		console.log("exhausted buffers!");
		return null;
	}

	var outputBuffer = outputBuffers.shift();
	
	var currentTime = samplePosition / sampleRate;
	
	var chunkOffset = 0;
	while (chunkOffset < BUFFER_SIZE) {
		host.processAudioChunk(instancePtr, messages, inputBuffer, outputBuffer, currentTime, sampleRate, chunkOffset);
		messages = [];
		chunkOffset += CHUNK_SIZE;
	}
	
	samplePosition += BUFFER_SIZE;

	return outputBuffer;
}

function processSequencer(numSamples) {
	// schedule web audio node events at this number of samples
	// chunk into samples per tick

	var currentSamplePosition = samplePosition;
	while (numSamples > 0) {
		if (currentTickPosition == 0) {
		
			project.sequenceTracks.forEach(function(sequenceTrack) {
				var sequenceEvent = sequenceTrack.getEventAt(currentTick);
				if (sequenceEvent) {
					sequenceTrack.playingRow = 0;
					sequenceTrack.playingPattern = sequenceTrack.patterns[sequenceEvent.patternIndex];
					//console.log("playing pattern " + sequenceEvent.patternIndex);
				}
				
				if (sequenceTrack.playingPattern) {
					// schedule all pattern events at sequenceTrack.playingRow across all columns
					for (var i = 0; i < sequenceTrack.columns.length; i++) {
						var column = sequenceTrack.columns[i];
						var patternEvent = sequenceTrack.playingPattern.getEventAt(column.vertex, column.pin, sequenceTrack.playingRow);
						if (patternEvent) {
							setParameterValue(sequenceTrack, patternEvent.vertex, patternEvent.pin, patternEvent.value, currentSamplePosition);
						}
					}

					sequenceTrack.playingRow ++;
					if (sequenceTrack.playingRow >= sequenceTrack.playingPattern.rows) {
						sequenceTrack.playingPattern = null;
					}
				}
			});
		}
		
		var maxSamples = Math.min(samplesPerTick - currentTickPosition, numSamples);

		currentTickPosition += maxSamples;
		if (currentTickPosition >= samplesPerTick) {
			currentTickPosition = 0;
			currentTick ++;
		}
		numSamples -= maxSamples;
		currentSamplePosition += maxSamples;
		
		if (currentTick >= project.sequenceLength) {
			currentTick = 0;
		}
	}	
	
	
	function setParameterValue(sequenceTrack, vertex, pin, value, timestamp) {

		// patterns/sequence tracks refer to columns by vertex+pin name, but the audio engine refers to top level propagated pins
		// look up the propagated pin, and set the parameter's value through it

		// NOTE: this uses the uncompiled graph because LinxHost's compiled pins parser dont extract the original pin name
		
		var pinref = project.graph.propagatedPinRefs.find(function(pinref) {
			// the patterns "pin" is the vertex' underlying pin name. pinref.name is the overriden name, but is ignored here
			return pinref.vertex == vertex && pinref.pin == pin;
		});
		
		if (pinref) {
			var pinIndex = project.graph.propagatedPinRefs.indexOf(pinref);
			var hostPin = host.pins[pinIndex]; // the host map compiled pins 1:1 to the project graphs propagated pins
			if (hostPin.pin.type == 4) {
				var messages = sequenceTrack.transformer.transformToMidi(value, project.key, project.scale);
				messages.forEach(function(message) {
					queueParameterValue(pinIndex, message, timestamp);
				});
			} else {
				queueParameterValue(pinIndex, value, timestamp);
			}
		} else {
			console.log(vertex.name, pin, "is not propagated");
		}
		//	console.log("just did: ", sequenceTrack, columnName, repeatIndex, value, timestamp);
	}

	function queueParameterValue(pinIndex, value, timestamp) {
		messages.push({pinIndex : pinIndex, value : value, timestamp : timestamp});
	}

}

function processNote(graphId, vertex, pinName, note, velocity) {
	var currentGraph = project.getGraphByIdentifier(graphId);// project.graph;
	var projectVertex = currentGraph.getVertex(vertex);
	var subgraphInfo = resolveSubgraphInstanceVertexIndex(currentGraph, projectVertex);		
	var pinInfo = projectVertex.resolvePin(pinName);
	var channel = 0; // TODO: ?		
	var value = Module._midi_make(channel, 9, note, velocity);
	//console.log(subgraphInfo.graphPtr + ", " + subgraphInfo.vertexIndex + " " + note + " " + velocity);
	Module._linx_graph_instance_process_vertex_midi(subgraphInfo.graphPtr, subgraphInfo.vertexIndex, pinInfo.pinIndex, pinInfo.pinGroup, value);
}

function parameterChange(graphId, vertex, pinName, value) {	
	// NOTE: setting parameters on vertices in a running graph is a low level operation that
	// needs to synchronise graph/vertex objects on the project side with C pointers on the
	// host side.
	// the entire project is compiled to a graph definition, from which a graph instance is 
	// created. the instance is usually treated as an opaque C pointer, and values are set 
	// only on propagated pins passed through the global graph interface. 
	// however during editing we want to set/queue values on pins on individual vertices in 
	// subgraphs, which also become the respective initial value in instances of the currently 
	// edited graph.

	var currentGraph = project.getGraphByIdentifier(graphId);// project.graph;
	var projectVertex = currentGraph.getVertex(vertex);
	var subgraphInfo = resolveSubgraphInstanceVertexIndex(currentGraph, projectVertex);
	var instance = subgraphInfo.graphPtr;
	var vertexIndex = subgraphInfo.vertexIndex;		
	var pinInfo = projectVertex.resolvePin(pinName);
	var pinIndex = pinInfo.pinIndex;
	var pinGroup = pinInfo.pinGroup;
	var pin = pinInfo.resolvedPin;

	console.log("parameterChange", vertex, pinName, vertexIndex, pinIndex, pinGroup, value);

	if (pin.type == 0) {
		Module._linx_graph_instance_process_vertex_float(instance, vertexIndex, pinIndex, pinGroup, value);
	} else if (pin.type == 2) {
		Module._linx_graph_instance_process_vertex_int(instance, vertexIndex, pinIndex, pinGroup, value);
	}
	
	// set init value on the underlying vertex: never set propagated init values
	pinInfo.resolvedVertex.setInitValue(pinInfo.resolvedPin.name, value);
	
	// update projectVertex.values -> name:value pair for persistent init value, remove if default
	// projectVertex.setInitValue(pinName, value);		
}

function describeValue(graphId, vertexId, pinName, value) {
	// NOTE: host also has a describevalue but is a high level describer, this low level resolves any pin on any vertex in the current graph
	var currentGraph = project.getGraphByIdentifier(graphId);
	var projectVertex = currentGraph.getVertex(vertexId);
	var subgraphInfo = resolveSubgraphInstanceVertexIndex(currentGraph, projectVertex);
	var instance = subgraphInfo.graphPtr;
	var vertexIndex = subgraphInfo.vertexIndex;
	var pinInfo = projectVertex.resolvePin(pinName);
	var pinIndex = pinInfo.pinIndex;
	var pinGroup = pinInfo.pinGroup;
	
	// stack alloc string for result
	var stack = Runtime.stackSave();
	var descriptionPtr = Runtime.stackAlloc(1024);
	Module._linx_graph_instance_describe_vertex_value(instance, vertexIndex, pinIndex, pinGroup, value, descriptionPtr, 1024);
	var description = AsciiToString(descriptionPtr); 
	Runtime.stackRestore(stack);

	if (description.length) {
		return description;
	} else {
		return value.toString();
	}
}

function resolveSubgraphInstanceVertexIndex(graph, vertex) {
	if (graph.parent) {
	
		var result = resolveSubgraphInstanceVertexIndex(graph.parent, graph.parentVertex);
		var parentVertexPtr = Module._linx_graph_instance_get_vertex_instance(result.graphPtr, result.vertexIndex);
		var subgraphPtr = Module._linx_vertex_instance_get_subgraph(parentVertexPtr);
		var index = graph.vertices.indexOf(vertex);
		return { graphPtr: subgraphPtr, vertexIndex : index };
	} else {
		// at the root, 
		var index = graph.vertices.indexOf(vertex);
		return { graphPtr: instancePtr, vertexIndex : index };
	}		
}

function getEdgeDigest(graphId) {
	if (host == null) {
		return null;
	}

	// set message.buffer to digest, scale raw values to -1..1 here, because the graph doesnt know anything about pin ranges
	var currentGraph = project.getGraphByIdentifier(graphId);
	//var currentGraph = project.graph;
	var edgeCount = currentGraph.edges.length;
	var edgeDigestSise = 32;
	var subgraphInfo = resolveSubgraphInstanceVertexIndex(currentGraph, null);
	
	var bufferPtr = Module._linx_graph_definition_get_snapshot(subgraphInfo.graphPtr);

	var result = new Float32Array(edgeCount * edgeDigestSise);
	
	currentGraph.edges.forEach(function(edge, edgeIndex) {
	
		var pinInfo = edge.sourceVertex.resolvePin(edge.from_pin);
		if (!pinInfo)  {
			// subgraph pin or something
			// console.log(edge.from_pin);
			return ;
		}

		var sourcePin = pinInfo.resolvedPin;
		var sourceRange = sourcePin.maxValue - sourcePin.minValue;
		
		var buffer = new Float32Array(Module.HEAPU8.buffer, bufferPtr + edgeIndex * edgeDigestSise * 4, edgeDigestSise);
		for (var i = 0; i < edgeDigestSise; i++) {
			var value = 0;
			
			if (sourcePin.type == 1 || sourcePin.type == 3) {
				value = ((buffer[i] - sourcePin.minValue) / sourceRange) * 2 - 1;
			} else {
				value = buffer[i];
			}
		
			result[edgeIndex * edgeDigestSise + i] = value;
		}
	});
	
	return result;
}
