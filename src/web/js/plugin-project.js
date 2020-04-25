
function LinxPatternEvent(vertex, pin, time, value) {
	this.vertex = vertex; // object
	this.pin = pin; // name
	this.time = time;
	this.value = value;
}

function LinxPattern(name, rows) {
	this.name = name;
	this.rows = rows;
	this.events = [];
}

LinxPattern.prototype.getEventAt = function(vertex, pin, time) {
	return this.events.find(function(ev) {
		return ev.vertex == vertex && ev.pin == pin && ev.time == time;
	});
}

function LinxSequenceEvent(time, patternIndex) {
	this.time = time;
	this.patternIndex = patternIndex;
}

function LinxSequenceColumn(vertex, pin) {
	this.vertex = vertex;
	this.pin = pin;
}

function LinxSequenceTrack() {
	this.columns = [];
	this.sequenceEvents = [];
	this.patterns = [];

	// NOTE/TODO: playback state only used on worker side
	if (typeof MidiNoteTransformer == "function") {
		this.playingRow = 0;
		this.playingPattern = null;
		this.transformer = new MidiNoteTransformer();
	}
}

LinxSequenceTrack.prototype.getColumn = function(vertex, pin) {
	return this.columns.find(function(c) {
		return c.vertex == vertex && c.pin == pin;
	});
}

LinxSequenceTrack.prototype.getPattern = function(name) {
	return this.patterns.find(function(pattern) {
		return pattern.name == name;
	});
}

LinxSequenceTrack.prototype.getNewPatternName = function() {
	var counter = 0;
	var name;
	while (true) {
		name = counter.toString();
		while (name.length < 2) {
			name = "0" + name;
		}
		if (!this.getPattern(name)) {
			break;
		}
		counter ++;
	}
	return name;
}

LinxSequenceTrack.prototype.getEventAt = function(time) {
	return this.sequenceEvents.find(function(ev) {
		return ev.time == time;
	});
}

LinxSequenceTrack.prototype.setEventAt = function(time, value) {
	var ev = this.getEventAt(time);
	if (ev) {
		if (value == null) {
			var eventIndex = this.sequenceEvents.indexOf(ev);
			this.sequenceEvents.splice(eventIndex, 1);
		} else {
			ev.patternIndex = value;
		}
	} else if (value != null) {
		//console.log("new event @ " + time + " =  " + value);
		this.sequenceEvents.push(new LinxSequenceEvent(time, value));
	}
}

LinxSequenceTrack.prototype.shiftSequence = function(cursorTime, deltaTime) {
	for (var i = 0; i < this.sequenceEvents.length; i++) {
		var sequenceEvent = this.sequenceEvents[i];
		if (sequenceEvent.time >= cursorTime) {
			sequenceEvent.time += deltaTime;
		}
	}
}


function LinxFactory(factoryPtr, name, symbol, file, deps, pins, subgraphPins, isSubgraphParent, category, description) {
	this.factoryPtr = factoryPtr;
	this.name = name;
	this.symbol = symbol;
	this.file = file;
	this.deps = deps;
	this.pins = pins;
	this.subgraphPins = subgraphPins;
	this.isSubgraphParent = isSubgraphParent;
	this.category = category;
	this.description = description;
}

LinxFactory.prototype.getPinIndex = function(name) {
	for (var i = 0; i < this.pins.length; i++) {
		var pin = this.pins[i];
		if (pin.name == name) {
			return i;
		}
	}
	return -1;
}

LinxFactory.prototype.getSubgraphPinIndex = function(name) {
	for (var i = 0; i < this.subgraphPins.length; i++) {
		var pin = this.subgraphPins[i];
		if (pin.name == name) {
			return i;
		}
	}
	return -1;
}

LinxFactory.prototype.isPinCompatible = function(pin, sourcePin) {
	if (sourcePin.type == 1 && pin.type == 0) {
		// float -> float
		return true;
	} else if (sourcePin.type == 1 && pin.type == 2) {
		// float -> int
		return true;
	} else if (sourcePin.type == 3 && pin.type == 2) {
		// int -> int
		return true;
	} else if (sourcePin.type == 3 && pin.type == 0) {
		// int -> float
		return true;
	} else if (sourcePin.type == 7 && pin.type == 6) {
		// float buffer -> float buffer
		return true;
	} else if (sourcePin.type == 5 && pin.type == 4) {
		// midi -> midi
		return true;
	}
	return false;
}

function LinxFactoryPin() {
	this.name = "";
	this.type = "";
	this.minValue = 0;
	this.maxValue = 0;
	this.defaultValue = 0;
	this.precision = 0;
}

function LinxPinRef() {
	this.name = "";
	this.vertex = null;
	this.pin = "";
}

function LinxVertex() {
	this.name = "";
	this.factory = null;
	this.subgraph = null;
	this.values = [];
	this.x = 0;
	this.y = 0;
}

function LinxPinInfo() {
	this.pinGroup = 0;
	this.pinIndex = 0;
	this.resolvedPin = null;
	this.resolvedVertex = null;
}

LinxVertex.prototype.resolvePin = function(pinName) {
	var pinIndex = this.factory.getPinIndex(pinName);
	if (pinIndex != -1) {
		var result = new LinxPinInfo();
		result.pinGroup = 0;
		result.pinIndex = pinIndex;
		result.resolvedPin = this.factory.pins[pinIndex];
		result.resolvedVertex = this;
		return result;
	} else if (this.factory.isSubgraphParent) {
		var subgraphPinrefIndex = this.subgraph.getPropagatedPinRefIndex(pinName);
		if (subgraphPinrefIndex != -1) {
			var result = new LinxPinInfo();
			result.pinGroup = 1;
			result.pinIndex = subgraphPinrefIndex;
			
			var pinref = this.subgraph.propagatedPinRefs[subgraphPinrefIndex];
			var resolvedPinInfo = pinref.vertex.resolvePin(pinref.pin);
			if (!resolvedPinInfo) {
				return null;
			}

			result.resolvedPin = resolvedPinInfo.resolvedPin;
			result.resolvedVertex = resolvedPinInfo.resolvedVertex;
			return result;
		}
	}
	return null;
}

LinxVertex.prototype.resolveParentPin = function(pinName) {
	var pinIndex = this.factory.getSubgraphPinIndex(pinName);
	if (pinIndex != -1) {
		var result = new LinxPinInfo();
		result.pinGroup = 0;
		result.pinIndex = pinIndex;
		// TODO: rsolvedPin, resolvedVertex?
		return result;
	}
	return null;
}

LinxVertex.prototype.getInitValue = function(pinName) {
	var pinInfo = this.resolvePin(pinName);
	if (!pinInfo) {
		console.log("invalid pin name");
		return 0;
	}

	if (pinInfo.resolvedVertex == this) {
		var vertexValue = this.values.find(function(value) {
			return value[0] == pinName;
		});
		
		if (vertexValue) {
			return vertexValue[1];
		}
		return pinInfo.resolvedPin.defaultValue;
	} else {
		return pinInfo.resolvedVertex.getInitValue(pinInfo.resolvedPin.name);
	}
	//return pinInfo.resolvedPin.defaultValue;

}

LinxVertex.prototype.setInitValue = function(pinName, value) {
	var pinInfo = this.resolvePin(pinName);
	if (!pinInfo) {
		console.log("cannot set init value for pin '" + pinName + "'");
		return ;
	}
	
	if (pinInfo.resolvedVertex == this) {
		var vertexValue = this.values.find(function(value) {
			return value[0] == pinName;
		});
		
		// TODO: remove if value == pin.defaultValue
		if (vertexValue) {
			vertexValue[1] = value;
		} else {
			this.values.push([pinName, value]);
		}
	} else {
		// does not set init value on propagated pin, init value must be set on source pin
	}
}

function LinxEdge() {
	this.sourceVertex = "";
	this.from_pin = "";
	this.targetVertex = "";
	this.to_pin = "";
}

function LinxGraph() {
	this.parent = null;
	this.parentVertex = null;
	this.vertices = [];
	this.edges = [];
	this.propagatedPinRefs = [];
}

LinxGraph.prototype.importVertex = function(vertex) {
	if (vertex.factory.isSubgraphParent) {
		vertex.subgraph.parent = this;
		vertex.subgraph.parentVertex = vertex;
	}
	this.vertices.push(vertex);
}

/*LinxGraph.prototype.addVertex = function(name, factory) {
	var vertex = new LinxVertex();
	vertex.name = name;
	vertex.factory = factory;
	if (factory.isSubgraphParent) {
		vertex.subgraph = new LinxGraph();
		vertex.subgraph.parent = this;
		vertex.subgraph.parentVertex = vertex;
	}
	this.vertices.push(vertex);
	return vertex;
}

LinxGraph.prototype.addEdge = function(targetVertex, to_pin, sourceVertex, from_pin) {
	var edge = new LinxEdge();
	edge.targetVertex = targetVertex;
	edge.to_pin = to_pin;
	edge.sourceVertex = sourceVertex;
	edge.from_pin = from_pin;
	this.edges.push(edge);
	return edge;
}

LinxGraph.prototype.addPin = function(name, vertex, pin) {
	var pinref = new LinxPinRef();
	pinref.name = name; // name of pin when exposed to host or parent graphs
	pinref.vertex = vertex; // vertex object
	pinref.pin = pin; // the pin name, pass to e.g resolvePin to get underlying pin
	this.propagatedPinRefs.push(pinref);
	return pinref;
}*/

LinxGraph.prototype.getVertex = function(name) {
	for (var i = 0; i < this.vertices.length; i++) {
		var vertex = this.vertices[i];
		if (vertex.name == name) {
			return vertex;
		}
	}
	return null;
}

LinxGraph.prototype.getPropagatedPinRefIndex = function(name) {
	for (var i = 0; i < this.propagatedPinRefs.length; i++) {
		var pinref = this.propagatedPinRefs[i];
		if (pinref.name == name) {
			return i;
		}
	}
	return -1;
}

/*LinxGraph.prototype.removeVertex = function(vertex) {	
	this.propagatedPinRefs = this.propagatedPinRefs.filter(function(pinref) {
		if (pinref.vertex == vertex) {
			// TODO: remove propagated pin in parent recursively, if propagated
			return false;
		}
		return true;
	});
	
	this.edges = this.edges.filter(function(edge) {
		return (edge.sourceVertex != vertex && edge.targetVertex != vertex)
	});

	this.vertices = this.vertices.filter(function(thisVertex) {
		return (thisVertex != vertex)
	});
}*/

LinxGraph.prototype.getNewVertexName = function(name) {
	var counter = 1;
	for (;;) {
		var testName;
		if (counter == 1) {
			testName = name
		} else {
			testName = name + counter;
		}
		if (!this.getVertex(testName)) {
			return testName;
		}
		counter ++;
	}
}

LinxGraph.prototype.resolveSubgraphInstanceVertexIndex = function(vertex) {
	if (this.parent) {
	
		var result = this.parent.resolveSubgraphInstanceVertexIndex(this.parentVertex);
		var parentVertexPtr = Module._linx_graph_instance_get_vertex_instance(result.graphPtr, result.vertexIndex);
		var subgraphPtr = Module._linx_vertex_instance_get_subgraph(parentVertexPtr);
		var index = this.vertices.indexOf(vertex);
		return { graphPtr: subgraphPtr, vertexIndex : index };
	} else {
		// at the root, 
		var index = this.vertices.indexOf(vertex);
		return { graphPtr: self.webAudioNode.instancePtr, vertexIndex : index };
	}		
}



function LinxProject(factories) {
	this.factories = factories;

	Events(this);

	this.clear();	
}

LinxProject.prototype.clear = function() {
	this.name = "";
	this.author = "";
	this.uniqueId = "NOID";
	this.myjsonId = "";
	this.graph = new LinxGraph();
	this.sequenceTracks = [];
	this.sequenceLength = 4096;
	this.bpm = 126;
	this.tpb = 4;
	this.key = 0;
	this.scale = "major";
	this.emit("clear");
}

LinxProject.prototype.getFactory = function(name) {
	for (var i = 0; i < this.factories.length; i++) {
		var factory = this.factories[i];
		if (factory.name == name) {
			return factory;
		}
	}
	return null;
}

LinxProject.prototype.getGraphByIdentifier = function(identifier) {
	var index = 0;
	return scan(this.graph);
	
	function scan(subgraph) {
		if (identifier == index) return subgraph;
		index++;
		
		for (var i = 0; i < subgraph.vertices.length; i++) {
			var vertex = subgraph.vertices[i];
			if (vertex.factory.isSubgraphParent) {
				var result = scan(vertex.subgraph);
				if (result != null) {
					return result;
				}
			}
		}
		return null;
	}
}

LinxProject.prototype.getGraphIdentifier = function(graph) {
	var index = 0;
	return scan(this.graph);
	
	function scan(subgraph) {
		//console.log("comparing ", graph == subgraph, graph === subgraph, graph, subgraph);
		if (graph == subgraph) return index;
		index++;

		for (var i = 0; i < subgraph.vertices.length; i++) {
			var vertex = subgraph.vertices[i];
			if (vertex.factory.isSubgraphParent) {
				var result = scan(vertex.subgraph);
				if (result != -1) {
					return result;
				}
			}
		}
		return -1;
	}
}

LinxProject.prototype.createSequenceTrack = function() {
	var sequenceTrack = new LinxSequenceTrack();
	this.sequenceTracks.push(sequenceTrack);
	this.emit("insertSequenceTrack", sequenceTrack);
	return sequenceTrack;
}

LinxProject.prototype.addSequenceTrackColumn = function(sequenceTrack, vertex, pin) {
	var column = new LinxSequenceColumn(vertex, pin);
	sequenceTrack.columns.push(column);
	this.emit("insertSequenceTrackColumn", sequenceTrack, column);
	return column;
}

LinxProject.prototype.createPattern = function(sequenceTrack, rows, name) {
	if (!sequenceTrack) throw new Error("Invalid argument: sequenceTrack");
	if (!rows) throw new Error("Invalid argument: rows");

	if (typeof(name) == "undefined") name = sequenceTrack.getNewPatternName();

	var pattern = new LinxPattern(name, rows);
	sequenceTrack.patterns.push(pattern);
	
	this.emit("insertPattern", sequenceTrack, pattern);
	return pattern;
}

LinxProject.prototype.setPatternEventAt = function(pattern, vertex, pin, time, value) {
	var ev = pattern.getEventAt(vertex, pin, time);
	console.log("setting", pin, time, value);
	if (ev) {
		if (value == null) {
			var eventIndex = pattern.events.indexOf(ev);
			pattern.events.splice(eventIndex, 1);
			this.emit("deletePatternEvent", ev);
		} else {
			ev.value = value;
			this.emit("updatePatternEvent", ev);
		}
	} else if (value != null) {
		pattern.events.push(new LinxPatternEvent(vertex, pin, time, value));
		this.emit("insertPatternEvent", ev);
	}
}

LinxProject.prototype.shiftPatternEvents = function(pattern, cursorTime, deltaTime) {

	for (var i = 0; i < pattern.events.length; i++) {
		var patternEvent = pattern.events[i];
		if (patternEvent.time >= cursorTime) {
			patternEvent.time += deltaTime;
			this.emit("updatePatternEvent", patternEvent);
		}
	}
}


LinxProject.prototype.addVertex = function(graph, name, factory, x, y) {
	var vertex = new LinxVertex();
	vertex.name = name;
	vertex.factory = factory;
	if (factory.isSubgraphParent) {
		vertex.subgraph = new LinxGraph();
		vertex.subgraph.parent = graph;
		vertex.subgraph.parentVertex = vertex;
	}

	if (typeof(x) != "undefined") vertex.x = x;
	if (typeof(y) != "undefined") vertex.y = y;

	graph.vertices.push(vertex);
	
	this.emit("insertVertex", graph, vertex);
	return vertex;
}

LinxProject.prototype.removeVertex = function(graph, vertex) {
	var self = this;
	graph.propagatedPinRefs = graph.propagatedPinRefs.filter(function(pinref) {
		if (pinref.vertex == vertex) {
			// TODO: remove propagated pin in parent recursively, if propagated
			return false;
		}
		//TODO: also for edges .. self.emit("removePinref", graph, vertex, pinref);
		return true;
	});
	
	graph.edges = graph.edges.filter(function(edge) {
		return (edge.sourceVertex != vertex && edge.targetVertex != vertex)
	});

	graph.vertices = graph.vertices.filter(function(thisVertex) {
		return (thisVertex != vertex)
	});
	
	this.emit("deleteVertex", graph, vertex);
}

LinxProject.prototype.renameVertex = function(vertex, name) {
	vertex.name = name;
	this.emit("updateVertex", vertex);
}

LinxProject.prototype.addEdge = function(graph, targetVertex, to_pin, sourceVertex, from_pin) {
	var edge = new LinxEdge();
	edge.targetVertex = targetVertex;
	edge.to_pin = to_pin;
	edge.sourceVertex = sourceVertex;
	edge.from_pin = from_pin;
	graph.edges.push(edge);
	
	this.emit("insertEdge", graph, edge);
	return edge;
}

LinxProject.prototype.removeEdge = function(graph, edge) {
	graph.edges = graph.edges.filter(function(e) {
		return e != edge;
	});
	this.emit("deleteEdge", edge);
}

LinxProject.prototype.removeEdges = function(graph, targetVertex, sourceVertex) {
	var self = this;
	var edges = graph.edges.filter(function(edge) {
		return (edge.sourceVertex == sourceVertex && edge.targetVertex == targetVertex);
	});
	edges.forEach(function(edge) {
		self.removeEdge(graph, edge);
	});
}

LinxProject.prototype.addPin = function(graph, name, vertex, pin) {
	var pinref = new LinxPinRef();
	pinref.name = name; // name of pin when exposed to host or parent graphs
	pinref.vertex = vertex; // vertex object
	pinref.pin = pin; // the pin name, pass to e.g resolvePin to get underlying pin
	graph.propagatedPinRefs.push(pinref);
	
	this.emit("insertPinref", pinref);
	return pinref;
}

LinxProject.prototype.deletePinref = function(graph, pinref) {
	graph.propagatedPinRefs = graph.propagatedPinRefs.filter(function(p) {
		return p != pinref;
	});
	this.emit("deletePinref", pinref);
}

LinxProject.prototype.deletePinrefs = function(graph) {
	var self = this;
	var pinrefs = graph.propagatedPinRefs.slice();
	pinrefs.forEach(function(p) {
		self.deletePinref(graph, p);
	});
}

LinxProject.prototype.parseJsonProject = function(objectProject) {
	// try to parse string as JSON
	if (typeof(objectProject) == "string") {
		try {
			objectProject = JSON.parse(objectProject);
		} catch (ex) {
			console.log(ex);
			return false;
		}
	}

	if (!objectProject.graph) {
		console.log("expected 'graph' object");
		return false;
	}

	if (!objectProject.system) {
		console.log("expected 'system' object");
		return false;
	}

	this.name = objectProject.system.product;
	this.author = objectProject.system.author;
	this.uniqueId = objectProject.system.uniqueId;
	this.myjsonId = objectProject.system.myjsonId;
	
	var objectSubgraphs = objectProject.subgraphs ? objectProject.subgraphs : [];
	
	if (!this.parseJsonGraph(null, this.graph, objectProject.graph, objectSubgraphs)) {
		return false;
	}
	
	this.parseJsonSequence(objectProject);
		
	return true;
}

LinxProject.prototype.parseJsonSequence = function(objectProject) {

	var self = this;

	if (!objectProject.sequence) {
		return ;
	}
	
	this.bpm = parseInt(objectProject.sequence.bpm);
	if (!this.bpm) this.bpm = 126;
	
	this.tpb = parseInt(objectProject.sequence.tpb);
	if (!this.tpb) this.tpb = 4;

	this.key = parseInt(objectProject.sequence.key);
	if (!this.key) this.key = 0;

	this.scale = objectProject.sequence.scale;
	if (!this.scale) this.scale = "major";

	// TODO: does not import correctly sequences for renamed vertices

	var sequenceRows = parseInt(objectProject.sequence.length);
	this.sequenceLength = sequenceRows;
	
	objectProject.sequence.tracks.forEach(function(objectSequenceTrack) {

		if (typeof(objectSequenceTrack.columns) == "undefined") {
			return;
		}

		if (typeof(objectSequenceTrack.patterns) == "undefined") {
			return;
		}
	
		var sequenceTrack = self.createSequenceTrack();
		
		objectSequenceTrack.columns.forEach(function(objectColumn) {
			var vertex = self.graph.getVertex(objectColumn.vertex);
			self.addSequenceTrackColumn(sequenceTrack, vertex, objectColumn.pin);
		});
		
		objectSequenceTrack.patterns.forEach(function(objectPattern) {
			var patternRows = parseInt(objectPattern.rows);
			var pattern = self.createPattern(sequenceTrack, patternRows, objectPattern.name);

			objectPattern.events.forEach(function(objectEvent) {
				var vertex = self.graph.getVertex(objectEvent.vertex);
				
				self.setPatternEventAt(pattern, vertex, objectEvent.pin, objectEvent.time, objectEvent.value);
				
			});
		});
		
		if (objectSequenceTrack.events) {
			objectSequenceTrack.events.forEach(function(objectSequenceEvent) {
				sequenceTrack.setEventAt(objectSequenceEvent.time, objectSequenceEvent.patternIndex);
			});
		} else {
			console.log("no sequence events");
		}

	});
}

LinxProject.prototype.parseJsonGraph = function(parentVertex, graph, objectGraph, objectSubgraphs) {
	if (!objectGraph.vertices) {
		console.log("expected 'vertices' array");
		return false;
	}

	if (!objectGraph.edges) {
		console.log("expected 'edges' array");
		return false;
	}

	if (!objectGraph.pins) {
		console.log("expected 'pins' array");
		return false;
	}

	// keep track of vertices by their original name, in case renamed during import
	var importVertexMap = {};
	
	var d = 1 / objectGraph.vertices.length * Math.PI * 2;
	for (var i = 0; i < objectGraph.vertices.length; i++) {
		var objectVertex = objectGraph.vertices[i];
		if (!objectVertex.name) {
			console.log("invalid vertex name '" + objectVertex.name + "'");
			return false;
		}
		
		var vertexName;
		if (graph.getVertex(objectVertex.name)) {
			vertexName = graph.getNewVertexName(objectVertex.name);
		} else {
			vertexName = objectVertex.name;
		}
		
		var factory = this.getFactory(objectVertex.factory);
		if (!factory) {
			console.log("invalid vertex factory " + objectVertex.factory);
			return false;
		}
		
		var subgraph = null;
		var vertex = this.addVertex(graph, vertexName, factory);
		
		vertex.x = objectVertex.x || Math.sin(i * d) * 0.5;
		vertex.y = objectVertex.y || Math.cos(i * d) * 0.5;
		
		importVertexMap[objectVertex.name] = vertex;

		if (objectVertex.subgraph && factory.isSubgraphParent) {
			var objectSubgraph = objectSubgraphs.find(function(objectSubgraph) {
				return objectSubgraph.name == objectVertex.subgraph;
			});

			if (!objectSubgraph) {
				console.log("invalid subgraph " + objectVertex.subgraph);
				return false;
			}

			if (!this.parseJsonGraph(vertex, vertex.subgraph, objectSubgraph, objectSubgraphs)) {
				return false;
			}
		}

		if (objectVertex.values) {
			for (var valueKey in objectVertex.values) {
				vertex.setInitValue(valueKey, objectVertex.values[valueKey]);
			}
		}
	}
	
	for (var i = 0; i < objectGraph.edges.length; i++) {
		var objectEdge = objectGraph.edges[i];
		
		var sourceVertex;
		var targetVertex;

		if (objectEdge.from_vertex == "PARENT") {
			sourceVertex = parentVertex;
		} else {
			sourceVertex = importVertexMap[objectEdge.from_vertex]; //graph.getVertex(objectEdge.from_vertex);
			if (!sourceVertex) {
				console.log("invalid edge source: " + objectEdge.from_vertex);
				return false;
			}
		}
		
		if (objectEdge.to_vertex == "PARENT") {
			targetVertex = parentVertex;
		} else {
			targetVertex = importVertexMap[objectEdge.to_vertex]; //graph.getVertex(objectEdge.to_vertex);
			if (!targetVertex) {
				console.log("invalid edge target: " + objectEdge.to_vertex);
				return false;
			}
		}

		this.addEdge(graph, targetVertex, objectEdge.to_pin, sourceVertex,  objectEdge.from_pin);
	}
	
	for (var i = 0; i < objectGraph.pins.length; i++) {
		var objectPin = objectGraph.pins[i];
		
		var vertex = importVertexMap[objectPin.vertex]; //graph.getVertex(objectPin.vertex);
		if (!vertex) {
			console.log("invalid graph pin vertex reference: " + objectPin.vertex);
			return false;
		}
		
		this.addPin(graph, objectPin.name, vertex, objectPin.pin);
	}
	return true;
}

LinxProject.prototype.saveJsonProject = function() {

	var objectSubgraphs = [];
	var objectFactories = [];
	
	return {
		"system": {
			"uniqueId": this.uniqueId,
			"product": this.name,
			"author": this.author,
			"myjsonId" : this.myjsonId
		},
		"factories" : objectFactories,
		"targets" : [
			{ 
				"name" : "plain-win32", "arch" : "i386", "postfix" : ".dll",
				"deps" : [ "math_x86.obj", "graph.obj", "ftol2.obj", "chkstk.obj", "memcpy.obj", "memset.obj" ], 
				"exports" : [ "linx_host_get_graph" ], 
				"paths": [ "lib/cl-x86" ]
			},
			{ 
				"name" : "vst-win32", "arch" : "i386", "postfix" : "VST.dll",
				"deps" : [ "vsthost.obj", "commonhost.obj", "math_x86.obj", "graph.obj", "ftol2.obj", "chkstk.obj", "memcpy.obj", "memset.obj" ], 
				"exports" : [ "VSTPluginMain", "linx_host_get_graph" ],
				"paths": [ "lib/cl-x86" ]
			},
			{ 
				"name" : "vst-win64", "arch" : "x64", "postfix" : "VST64.dll",
				"deps" : [ "vsthost.obj", "commonhost.obj", "math_sse.obj", "fminfmax.obj",  "graph.obj", "chkstk.obj", "memcpy.obj", "memset.obj" ], 
				"exports" : [ "VSTPluginMain", "linx_host_get_graph" ], 
				"paths": [ "lib/cl-x64" ]
			},
			{
				"name" : "buzz-win32", "arch" : "i386", "postfix" : "Buzz.dll", 
				"deps" : [ "buzzhost.obj", "commonhost.obj", "math_x86.obj", "graph.obj", "ftol2.obj", "chkstk.obj", "memcpy.obj", "memset.obj", "thiscall.obj", "mman.obj" ], 
				"exports" : [ "GetInfo", "CreateMachine", "linx_host_get_graph" ], 
				"paths": [ "lib/cl-x86" ]
			}
		],
		"graph" : this.saveJsonGraph(this.graph, objectSubgraphs, objectFactories, null),
		"subgraphs" : objectSubgraphs,
		"sequence" : this.saveJsonSequence()
	};
}

LinxProject.prototype.saveJsonSequence = function() {
	var result = [];
	this.sequenceTracks.forEach(function(sequenceTrack) {
	
		var columns = [];
		for (var i = 0; i < sequenceTrack.columns.length; i++) {
			var column = sequenceTrack.columns[i];
			columns.push({
				vertex : column.vertex.name,
				pin : column.pin
			});
		}

		// TODO: pattern events have vertex references now
		var patterns = [];
		for (var i = 0; i < sequenceTrack.patterns.length; i++) {
			var pattern = sequenceTrack.patterns[i];
			patterns.push(savePattern(pattern));
		}
		
		result.push({
			columns : columns,
			events : sequenceTrack.sequenceEvents,
			patterns : patterns
		});
	});

	return {
		"bpm" : this.bpm,
		"tpb" : this.tpb,
		"key" : this.key,
		"scale" : this.scale,
		"length" : this.sequenceLength,
		"tracks" : result
	};
	
	function savePattern(pattern) {
		var events = [];
		for (var i = 0; i < pattern.events.length; i++) {
			var e = pattern.events[i];
			events.push({
				vertex : e.vertex.name,
				pin : e.pin,
				time : e.time,
				value : e.value
			});
		}
		return {
			name : pattern.name,
			rows : pattern.rows,
			events : events
		};
	}
}

LinxProject.prototype.saveJsonGraph = function(graph, objectSubgraphs, objectFactories, saveVertices) {
	// return this object, and add any subgraphs to objectSubgraphs and give them names
	var self = this;

	var objectVertices = [];
	graph.vertices.forEach(function(vertex) {
		if (saveVertices && saveVertices.indexOf(vertex) == -1) {
			return ;
		}
		
		var objectSubgraphName = null;
		if (vertex.factory.isSubgraphParent) {
			// NOTE: reserving slot in output subgraphs array before parsing recursively
			var objectSubgraphIndex = objectSubgraphs.length;
			objectSubgraphName = "subgraph" + objectSubgraphIndex;
			objectSubgraphs[objectSubgraphIndex] = null;

			var objectSubgraph = self.saveJsonGraph(vertex.subgraph, objectSubgraphs, objectFactories, null);
			objectSubgraph.name = objectSubgraphName;
			objectSubgraphs[objectSubgraphIndex] = objectSubgraph;
		}
		
		var objectValues = {};
		vertex.values.forEach(function(value) {
			// vertex.values is an array of arrays with two elements [key, value]
			objectValues[value[0]] = value[1];
		});
		
		var objectFactory = objectFactories.find(function(objectFactory) {
			return objectFactory.name == vertex.factory.name;
		});

		if (!objectFactory) {
			objectFactory = { name : vertex.factory.name, symbol : vertex.factory.symbol, file : vertex.factory.file, deps : vertex.factory.deps };
			objectFactories.push(objectFactory);
		}
		
		objectVertices.push({
			name : vertex.name,
			factory : vertex.factory.name,
			subgraph : objectSubgraphName,
			values : objectValues,
			x : vertex.x,
			y : vertex.y
		});
	});

	var objectEdges = [];
	graph.edges.forEach(function(edge) {
		if (saveVertices && (saveVertices.indexOf(edge.sourceVertex) == -1 || saveVertices.indexOf(edge.targetVertex) == -1)) {
			return ;
		}
		
		var sourceName = edge.sourceVertex == graph.parentVertex ? "PARENT" : edge.sourceVertex.name;
		var targetName = edge.targetVertex == graph.parentVertex ? "PARENT" : edge.targetVertex.name;
		objectEdges.push({
			from_vertex : sourceName,
			from_pin : edge.from_pin,
			to_vertex : targetName,
			to_pin : edge.to_pin
		});
	});
	
	var objectPins = [];
	graph.propagatedPinRefs.forEach(function(pinref) {
		if (saveVertices && saveVertices.indexOf(pinref.vertex) == -1) {
			return ;
		}
		objectPins.push({
			name : pinref.name,
			vertex : pinref.vertex.name,
			pin : pinref.pin
		});
	});
	
	return {
		vertices : objectVertices,
		edges : objectEdges,
		pins : objectPins
	};
}
