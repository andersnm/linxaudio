/*
LinxHost - Low level host functions for working with a root graph on the C side
*/
var CHUNK_SIZE = 256; // var linx_max_buffer_size = 1024;

function LinxHostPin(name, pin) {
	this.name = name;
	this.pin = pin;
}

function LinxHost(graphPtr) {
	this.inAudioPinIndexes = [];
	this.outAudioPinIndexes = [];
	this.inControlPinIndexes = [];
	this.inMidiPinIndexes = [];
	this.graphPtr = graphPtr;
	this.pins = [];

	var stack = Runtime.stackSave();
	var resolvedVertDefPtr = Runtime.stackAlloc(4); // struct vertex_definition*
	var resolvedPinIndexPtr = Runtime.stackAlloc(4); // int*

	var pinrefCount = Module._linx_graph_definition_get_pinref_count(graphPtr);
	for (var i = 0; i < pinrefCount; i++) {
	
		var pinrefPtr = Module._linx_graph_definition_get_pinref(graphPtr, i);
		var namePtr = Module._linx_pinref_get_name(pinrefPtr);
		var pinPtr = Module._linx_graph_definition_resolve_pin(graphPtr, i, resolvedVertDefPtr, resolvedPinIndexPtr);

		var pin = parseFactoryPin(pinPtr);
		var hostPin = new LinxHostPin(AsciiToString(namePtr), pin);
		this.pins.push(hostPin);
		
		// fetch propagated pin name and categorise on type --> eliminate this.parseHostParameterGroups() & co
		if (pin.type == 0 || pin.type == 2) {
			this.inControlPinIndexes.push(i);
		} else if (pin.type == 1 || pin.type == 3) {
			this.outControlPinIndexes.push(i);
		} else if (pin.type == 4) {
			this.inMidiPinIndexes.push(i);
		} else if (pin.type == 6) {
			this.inAudioPinIndexes.push(i);
		} else if (pin.type == 7) {
			this.outAudioPinIndexes.push(i);
		} else {
			console.log("cannot categorise pin type " + pin.type);
		}
	}
	
	Runtime.stackRestore(stack);
}

LinxHost.prototype.processAudioChunk = function(instancePtr, messages, input, output, currentTime, sampleRate, chunkOffset) {
	var self = this;
	function setAudioInputPinValues(instancePtr, input, inBufferPtr, chunkOffset) {
		var maxPeak = 0;
		for (var i = 0; i < self.inAudioPinIndexes.length; i++) {
			var pinrefIndex = self.inAudioPinIndexes[i];
			var pinGroup = 1; // public/propagated

			var inputBuffer = input.channels[i];
			//var inputBuffer = input.getChannelData(i);
			var inPinBufferPtr = inBufferPtr + (i * CHUNK_SIZE * 4);
			var buffer = new Float32Array(Module.HEAPU8.buffer, inPinBufferPtr, CHUNK_SIZE);
			for (var j = 0; j < buffer.length; j++) {
				buffer[j] = inputBuffer[j + chunkOffset];
				maxPeak = Math.max(buffer[j], maxPeak);
			}
			//console.log("setting float " + i + " maks " + maxPeak + " got " + inputBuffer.length + " pin " + pinrefIndex);
			Module._linx_graph_set_propagated_buffer(instancePtr, pinrefIndex, inPinBufferPtr, CHUNK_SIZE);
		}
	}
	
	function setPinValues(currentSamplePosition, messages, inValuesPtr) {
		for (var i = 0; i < messages.length; i++) {
			var message = messages[i];
			setPinRawValue(message.pinIndex, message.value, message.timestamp - currentSamplePosition, inValuesPtr);
		}
	}

	function setPinRawValue(pinrefIndex, value, timestamp, inValuesPtr) {
		if (timestamp < 0) {
			console.log("Ignoring netgative timestamp");
			return false;
		}

		var hostPin = self.pins[pinrefIndex];
		var pin = hostPin.pin;

		var pinIndex = pinrefIndex;
		var pinGroup = 1;
		
		if (pin.type == 2) {
			// linx_pin_type_in_scalar_int = 2
			if (value < pin.minValue || value > pin.maxValue) {
				console.log("parameter value out of bounds for " + pin.name);
				return false;
			}
			Module._linx_value_array_push_int(inValuesPtr, pinrefIndex, pinGroup, parseInt(value), timestamp);
			return true;
		} else if (pin.type == 0) {
			// linx_pin_type_in_scalar_float = 0
			if (value < pin.minValue || value > pin.maxValue) {
				console.log("parameter value out of bounds for " + pin.name + " range: " + pin.minValue + "-" + pin.maxValue);
				return false;
			}
			Module._linx_value_array_push_float(inValuesPtr, pinrefIndex, pinGroup, parseFloat(value), timestamp);
			return true;
		} else if (pin.type == 4) {
			// linx_pin_type_in_midi = 4
			Module._linx_value_array_push_midi(inValuesPtr, pinrefIndex, pinGroup, value, timestamp);
			return true;
		} else {
			console.log("cannot set value for this type " + pin.type);
			return false;
		}
	}
	
	function getAudioOutputBuffers(instancePtr, output, outBufferPtr, chunkOffset) {
		var maxPeak = 0;
		for (var i = 0; i < self.outAudioPinIndexes.length; i++) {
			var pinrefIndex = self.outAudioPinIndexes[i];

			var outPinBufferPtr = outBufferPtr + (i * CHUNK_SIZE * 4);
			if (Module._linx_graph_has_propagated_buffer(instancePtr, pinrefIndex)) {

				Module._linx_graph_get_propagated_buffer(instancePtr, pinrefIndex, outPinBufferPtr, CHUNK_SIZE);

				var buffer = new Float32Array(Module.HEAPU8.buffer, outPinBufferPtr, CHUNK_SIZE);
				var outputBuffer = output.channels[i];
				//var outputBuffer = output.getChannelData(i);
				for (var j = 0; j < buffer.length; j++) {
					outputBuffer[j + chunkOffset] = Math.max(Math.min(buffer[j], 0.99), -0.99);
					maxPeak = Math.max(outputBuffer[j + chunkOffset], maxPeak);
				}
			} else {
				var outputBuffer = output.channels[i];
				//var outputBuffer = output.getChannelData(i);
				for (var j = 0; j < CHUNK_SIZE; j++) {
					outputBuffer[j + chunkOffset] = 0;
				}
				maxPeak = 0;
			}
		}
	}

	if (instancePtr == null) {
		// is being deleted, dont process
		return ;
	}

	//var output = e.outputBuffer;
	//var input = e.inputBuffer;

	var stack = Runtime.stackSave();
	var outValuesDataPtr = Runtime.stackAlloc(1024 * 16); // linx_max_parameter_count * siseof(linx_value)
	var outValuesPtr = Runtime.stackAlloc(16); // siseof(linx_value_array)
	var inValuesDataPtr = Runtime.stackAlloc(1024 * 16); // linx_max_parameter_count * siseof(linx_value)
	var inValuesPtr = Runtime.stackAlloc(16); // siseof(linx_value_array)
	
	Module._linx_value_array_init_from(inValuesPtr, inValuesDataPtr, 0, 1024);
	Module._linx_value_array_init_from(outValuesPtr, outValuesDataPtr, 0, 1024);
	
	var inBufferPtr = Runtime.stackAlloc(CHUNK_SIZE * this.inAudioPinIndexes.length * 4); // 1k float samples per input pin
	var outBufferPtr = Runtime.stackAlloc(CHUNK_SIZE * this.outAudioPinIndexes.length * 4); // 1k float samples per input pin
	
	var currentSamplePosition = Math.floor(currentTime * sampleRate);
	setPinValues(currentSamplePosition, messages, inValuesPtr);

	setAudioInputPinValues(instancePtr, input, inBufferPtr, chunkOffset);
	Module._linx_graph_instance_process(instancePtr, 0, inValuesPtr, outValuesPtr, CHUNK_SIZE);

	//var outValueCount = Module.getValue(outValueCountPtr, "i32");
	
	getAudioOutputBuffers(instancePtr, output, outBufferPtr, chunkOffset);

	Module._linx_graph_instance_process_clear(instancePtr);
	
	Runtime.stackRestore(stack);

//	this.messages = [];

}

LinxHost.prototype.getParameterValue = function(instancePtr, pinIndex) {
	var hostPin = this.pins[pinIndex];
	var pin = hostPin.pin;
	if (pin.type == 0) {
		return Module._linx_graph_instance_get_current_value_float(instancePtr, pinIndex);
	} else if (pin.type == 2) {
		return Module._linx_graph_instance_get_current_value_int(instancePtr, pinIndex);
	} else {
		console.log("unexpected pin type");
		return 0;
	}
}

LinxHost.prototype.describeValue = function(instancePtr, pinIndex, value) {
	// stack alloc string for result
	var stack = Runtime.stackSave();
	var descriptionPtr = Runtime.stackAlloc(1024);

	Module._linx_graph_instance_describe_value(instancePtr, pinIndex, value, descriptionPtr, 1024);

	var description = AsciiToString(descriptionPtr); 
	if (!description.length) {
		description = value.toString();
	}

	Runtime.stackRestore(stack);
	return description;
}

LinxHost.prototype.destroy = function(instancePtr) {
	// NOTE: remains connected for a little while, must set instance to null to ensure its not used
	Module._linx_graph_instance_destroy(instancePtr);
	//this.instancePtr = null; 
}

LinxHost.prototype.stopNotes = function(messages, timestamp) {
	for (var i = 0; i < this.inMidiPinIndexes.length; i++) {
		var pinIndex = this.inMidiPinIndexes[i];
		var value = parseMidi(0);
		value.command = 0xb;
		value.data1 = 0x7b;
		messages.push({pinIndex : pinIndex, value : makeMidi(value), timestamp : timestamp});
	}
}
