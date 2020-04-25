function parseFactoryPin(pinPtr) {
	var pin = new LinxFactoryPin();
	var namePtr = Module._linx_pin_get_name(pinPtr);
	pin.name = AsciiToString(namePtr);
	pin.type = Module._linx_pin_get_type(pinPtr);
	// +x.toFixed("4") -> round to 4 decimal places via string, c->js numbers are a bit shaky
	pin.minValue = +Module._linx_pin_get_min(pinPtr).toFixed("4");
	pin.maxValue = +Module._linx_pin_get_max(pinPtr).toFixed("4");
	pin.defaultValue = +Module._linx_pin_get_default(pinPtr).toFixed("4");
	pin.precision = +Module._linx_pin_get_precision(pinPtr).toFixed("4");
	return pin;
}

function parseFactoryPins(factoryPtr) {
	var pinCount = Module._linx_factory_get_pin_count(factoryPtr);
	var result = [];
	for (var i = 0; i < pinCount; i++) {
		var pinPtr = Module._linx_factory_get_pin(factoryPtr, i);
		var pin = parseFactoryPin(pinPtr);
		result.push(pin);
	}
	return result;
}

function parseFactorySubgraphPins(factoryPtr) {
	var pinCount = Module._linx_factory_get_subgraph_pin_count(factoryPtr);
	var result = [];
	for (var i = 0; i < pinCount; i++) {
		var pinPtr = Module._linx_factory_get_subgraph_pin(factoryPtr, i);
		var pin = parseFactoryPin(pinPtr);
		result.push(pin);
	}
	return result;
}

function createFactories() {

	var linxModuleFactoryDefinitions = [
		{ "name" : "oscbuffer", "symbol": "oscbuffer_factory", "file": "oscbuffer.obj", "deps" : [ "inertia.obj" ],
			"category" : "Oscillator",
			"description" : "A simple oscillator. Supports generating basic sine, square, triangle, saw, noise waveforms."
		},
		{ "name" : "kickxp", "symbol": "kickxp_factory", "file": "kickxp.obj", "deps" : [ ],
			"category" : "Generator",
			"description" : "FSM KickXP - kick drum synthesis"
		},
		{ "name" : "container", "symbol": "container_factory", "file": "container.obj", "deps" : [ ],
			"category" : "Container",
			"description" : "A module container hosting a subgraph without any special processing. "
		},
		{ "name" : "polycontainer", "symbol": "polycontainer_factory", "file": "polycontainer.obj", "deps" : [ ],
			"category" : "Container",
			"description" : "Adds polyphonic support to the subgraph contents. Clones the subgraph for each voice up to 8 simultaneous voices."
		},
		{ "name" : "stereocontainer", "symbol": "stereocontainer_factory", "file": "stereocontainer.obj", "deps" : [ ],
			"category" : "Container",
			"description" : "Adds stereo support to subgraph contents. Clones the subgraph for the second channel."
		},
		{ "name" : "delay", "symbol": "delay_factory", "file": "delay.obj", "deps" : [ ],
			"category" : "Container",
			"description" : "A delay module using a subgraph to process the delay buffer."
		},
		{ "name" : "gain", "symbol": "gain_factory", "file": "gain.obj", "deps" : [ "inertia.obj"],
			"category" : "Amp/Dist",
			"description" : "A simple gain module."
		},
		{ "name" : "svf", "symbol": "svf_factory", "file": "svf.obj", "deps" : [ ],
			"category" : "Filter",
			"description" : "A simple state variable filter module."
		},
		{ "name" : "filter", "symbol": "biquad_filter_factory", "file": "biquad.obj", "deps" : [ ],
			"category" : "Filter",
			"description" : "A biquad filter module."
		},
		{ "name" : "trifilter", "symbol": "biquad_trifilter_factory", "file": "biquad.obj", "deps" : [ ],
			"category" : "Filter",
			"description" : "A port of the filter section from FSM Infector. Implements various hardwired combinations of the biquad eq and filter modules such as \"6L Multipeak\" and \"4L Skull D\"."
		},
		{ "name" : "eq", "symbol": "biquad_eq_factory", "file": "biquad.obj", "deps" : [ ],
			"category" : "Filter",
			"description" : "A biquad EQ module."
		},
		{ "name" : "distortion", "symbol": "distortion_factory", "file": "distortion.obj", "deps" : [ ],
			"category" : "Amp/Dist",
			"description" : "A distortion module."
		},
		{ "name" : "midinote", "symbol": "midinotesplit_factory", "file": "midinotesplit.obj", "deps" : [ ],
			"category" : "MIDI",
			"description" : "A MIDI event signal splitter."
		},
		{ "name" : "adsrvalue", "symbol": "adsrvalue_factory", "file": "adsrvalue.obj", "deps" : [ ],
			"category" : "Envelope",
			"description" : "An ADSR envelope value module."
		},
		{ "name" : "lfovalue", "symbol": "lfovalue_factory", "file": "lfovalue.obj", "deps" : [ ],
			"category" : "Envelope",
			"description" : "An LFO envelope value module."
		},
		{ "name" : "uservalue_u7", "symbol": "uservalue_u7_factory", "file": "uservalue.obj", "deps" : [ ] },
		{ "name" : "uservalue_u8", "symbol": "uservalue_u8_factory", "file": "uservalue.obj", "deps" : [ ] },
		{ "name" : "uservalue_f0_1", "symbol": "uservalue_f0_1_factory", "file": "uservalue.obj", "deps" : [ ] },
		{ "name" : "uservalue_fn1_1", "symbol": "uservalue_fn1_1_factory", "file": "uservalue.obj", "deps" : [ ] },
		{ "name" : "uservalue_hertz", "symbol": "uservalue_hertz_factory", "file": "uservalue.obj", "deps" : [ ] },
		{ "name" : "uservalue_ms", "symbol": "uservalue_ms_factory", "file": "uservalue.obj", "deps" : [ ] },
		//{ "name" : "transformvalue", "symbol": "transformvalue_factory", "file": "transformvalue.obj", "deps" : [ ] },
		{ "name" : "arithmeticvalue", "symbol": "arithmeticvalue_factory", "file": "arithmeticvalue.obj", "deps" : [ ] },
		{ "name" : "notefrequency", "symbol": "notefrequency_factory", "file": "notefrequency.obj", "deps" : [ ],
			"category" : "MIDI",
			"description" : "Converts MIDI notes to frequency."
		},
		{ "name" : "clip", "symbol": "clip_factory", "file": "clip.obj", "deps" : [ "inertia.obj" ],
			"category" : "Amp/Dist",
			"description" : "A clipping module."
		},
		{ "name" : "envscale", "symbol": "envscale_factory", "file": "envscale.obj", "deps" : [ ],
			"category" : "Envelope",
			"description" : "Scale values between two ranges."
		},
		//{ "name" : "phatman", "symbol": "phatman_factory", "file": "phatman.obj", "deps" : [ ] },
	];

	function createFactory(factoryDefinition) {
		// calling compiled asm.js function magic. assumes the module follows symbol naming conventions:
		// the "symbol" variable contains the C symbol name of the module factory struct. 
		// the struct symbol name is always written as the module's base name plus the string "_factory", 
		// f.ex "oscbuffer_factory". every module must also be accompanied with a C function which returns
		// the module factory. by convention this function is always written as the module's base name 
		// plus the string "_get_factory".
		//     var oscbuffer_factory = Module._oscbuffer_get_factory();
		
		var factoryFunctionName = "_" + factoryDefinition.symbol.replace(/factory/g, "get_factory");
		if (!Module[factoryFunctionName] || typeof(Module[factoryFunctionName]) != "function") {
			console.log("Cannot find factory function " + factoryFunctionName + ", " + typeof(Module[factoryFunctionName]));
		}
		
		var factoryPtr = Module[factoryFunctionName]();
		var pins = parseFactoryPins(factoryPtr);
		var subgraphPins = parseFactorySubgraphPins(factoryPtr);
		var isSubgraphParent = Module._linx_factory_is_subgraph_parent(factoryPtr);
		var category = factoryDefinition.category || "(not set)";
		var description = factoryDefinition.description || "";

		return new LinxFactory(factoryPtr, factoryDefinition.name, factoryDefinition.symbol, factoryDefinition.file, factoryDefinition.deps, pins, subgraphPins, isSubgraphParent, category, description);
	}
	
	var result = [];

	for (var i = 0; i < linxModuleFactoryDefinitions.length; i++) {
		var factoryDefinition = linxModuleFactoryDefinitions[i];
		var factory = createFactory(factoryDefinition);
		result.push(factory);
	}
	return result;
}

/*
function pinToString(pin) {
	var info = pin.name;
	if (pin.type == 0) {
		info += " (float input, range " + pin.minValue + " - " + pin.maxValue + ")";
	} else if (pin.type == 1) {
		info += " (float output, range " + pin.minValue + " - " + pin.maxValue + ")";
	} else if (pin.type == 2) {
		info += " (int input, range " + pin.minValue + " - " + pin.maxValue + ")";
	} else if (pin.type == 3) {
		info += " (int output, range " + pin.minValue + " - " + pin.maxValue + ")";
	} else if (pin.type == 4) {
		info += " (MIDI input)";
	} else if (pin.type == 6) {
		info += " (float buffer input)";
	} else if (pin.type == 7) {
		info += " (float buffer output)";
	} else {
		info += " (type " + pin.type + ")";
	}
	return info;
}

function dumpFactoryList(factoryList) {
	var info = "";
	for (var i = 0; i < factoryList.factories.length; i++) {
		var factory = factoryList.factories[i];
		info += factory.name + "<br />";
		for (var j = 0; j < factory.pins.length; j++) {
			var pin = factory.pins[j];
			info += "&nbsp;&nbsp;&nbsp;&nbsp;" + pinToString(pin);
			info +="<br />";
		}
	}
	return info;
}

function evaluateJavascriptSequencer(compiledNode, scriptText) {

	var stepSize = 0.2;
	function playNote(n, step, steps) {
		if (!steps) steps = 1;
		compiledNode.setMidiNote(n, 127, step * stepSize * context.sampleRate);
		compiledNode.setMidiNote(0, 0, (step + steps) * stepSize * context.sampleRate);
	}

	function interpolateParameter(parameterName, fromValue, toValue, step, steps) {
		var parameterIndex = compiledNode.getParameterIndexByName(parameterName);
		if (parameterIndex == -1) {
			console.log("no such parameter " + parameterName);
			return ;
		}
		
		var valueDelta = (toValue - fromValue) / steps;
		for (var i = 0; i < steps + 1; i++) {
			var value = fromValue + valueDelta * i;
			var timestamp = (step + i) * stepSize;
			compiledNode.setParameterValue(parameterIndex, value, timestamp * context.sampleRate);
			//console.log("value : " + value + ", time : " + timestamp);
		}
	}
	
	function setParameter(parameterName, value, step) {
		var parameterIndex = compiledNode.getParameterIndexByName(parameterName);
		if (parameterIndex == -1) {
			console.log("no such parameter " + parameterName);
			return ;
		}
		
		compiledNode.setParameterValue(parameterIndex, value, step * stepSize * context.sampleRate);
	}
	//console.log(sequence_textarea.value);
	eval(scriptText);

}
*/