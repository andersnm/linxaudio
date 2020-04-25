/*
LinxCompiler.compile() - compiles a LinxGraph instance to a new struct 
linx_graph_definition instance pointer.
*/
var LinxCompiler = { 
	compile : function(projectGraph) {
		// TODO: fix leaks on error
		var result = Module._linx_graph_definition_create(projectGraph.vertices.length, projectGraph.edges.length, projectGraph.propagatedPinRefs.length);
		for (var i = 0; i < projectGraph.vertices.length; i++) {
			var projectVertex = projectGraph.vertices[i];
			
			var subgraphPtr = null;
			if (projectVertex.subgraph) {
				subgraphPtr = LinxCompiler.compile(projectVertex.subgraph);
				if (!subgraphPtr) {
					return null;
				}
			}
			var vertexPtr = Module._graph_set_vertex(result, i, projectVertex.factory.factoryPtr, subgraphPtr, projectVertex.values.length);
			
			for (var j = 0; j < projectVertex.values.length; j++) {
				var projectValue = projectVertex.values[j];
				var pinIndex = projectVertex.factory.getPinIndex(projectValue[0]);
				var pin = projectVertex.factory.pins[pinIndex];
				if (!pin) {
					console.log("cannot set init value for " + projectValue[0]);
					continue;
				}
				if (pin.type == 2) {
					Module._graph_set_vertex_init_int(vertexPtr, j, pinIndex, projectValue[1]);
					//console.log("init value : " + pinIndex + " = " + projectValue[1]);
				} else if (pin.type == 0) {
					Module._graph_set_vertex_init_float(vertexPtr, j, pinIndex, projectValue[1]);
					//console.log("init value : " + pinIndex + " = " + projectValue[1]);
				} else {
					console.log("cannot init this type of value");
					return null;
				}
			}
			//console.log("compiled vertex");
		}

		for (var i = 0; i < projectGraph.edges.length; i++) {
			var projectEdge = projectGraph.edges[i];
			
			//var fromPinIndex;
			//var toPinIndex;
			var fromVertexIndex;
			var toVertexIndex;
			var fromPinInfo;
			var toPinInfo;
			
			if (projectEdge.sourceVertex == projectGraph.parentVertex) {
				//console.log("compiling edge to parent source");
				fromVertexIndex = -1;
				fromPinInfo = projectEdge.sourceVertex.resolveParentPin(projectEdge.from_pin);
			} else {
				fromPinInfo = projectEdge.sourceVertex.resolvePin(projectEdge.from_pin);
				//fromPinIndex = projectEdge.sourceVertex.factory.getPinIndex(projectEdge.from_pin);
				if (!fromPinInfo) {
					console.log("cant find from pin " + projectEdge.from_pin + " on vertex " + projectEdge.sourceVertex.name);
					return null;
				}
				
				fromVertexIndex = projectGraph.vertices.indexOf(projectEdge.sourceVertex);
			}

			if (projectEdge.targetVertex == projectGraph.parentVertex) {
				//console.log("compiling edge to parent target");
				toVertexIndex = -1;
				toPinInfo = projectEdge.targetVertex.resolveParentPin(projectEdge.to_pin);
			} else {
				toPinInfo = projectEdge.targetVertex.resolvePin(projectEdge.to_pin);
				//toPinIndex = projectEdge.targetVertex.factory.getPinIndex(projectEdge.to_pin);
				if (!toPinInfo) {
					console.log("cant find to pin " + projectEdge.to_pin + " on vertex " + projectEdge.targetVertex.name);
					return null;
				}
			
				toVertexIndex = projectGraph.vertices.indexOf(projectEdge.targetVertex);
			}

			/*console.log("adding edge: " + toVertexIndex + ", " + fromVertexIndex);
			console.log(projectEdge);
			console.log(fromPinInfo);
			console.log(toPinInfo);*/
			var resultEdge = Module._graph_set_edge(result, i, toVertexIndex, toPinInfo.pinIndex, toPinInfo.pinGroup, fromVertexIndex, fromPinInfo.pinIndex, fromPinInfo.pinGroup);
			//var resultEdge = Module._graph_set_edge(result, i, toVertexIndex, toPinIndex, 0, fromVertexIndex, fromPinIndex, 0);
			//console.log("compiled edge");// + toVertexIndex + ", " + toPinIndex + ", " + fromVertexIndex + ", " + fromPinIndex);
		}

		for (var i = 0; i < projectGraph.propagatedPinRefs.length; i++) {
			var projectPin = projectGraph.propagatedPinRefs[i];
			var vertex = projectPin.vertex;

			// lookup pin name in factory
			// TODO: name must stay alive for the lifetime of the compiled graph - api helper linx_free_name_ptrs() ??
			var namePtr = Module._malloc(projectPin.name.length + 1); // runtime const char*
			stringToAscii(projectPin.name, namePtr);
			var vertexIndex = projectGraph.vertices.indexOf(vertex);
			
			var pinInfo = vertex.resolvePin(projectPin.pin);
			
			//var pinIndex = vertex.factory.getPinIndex(projectPin.pin);
			if (!pinInfo) {
				console.log("cant find propagated pin for " + projectPin.pin + " on " + vertex.name);
				return null;
			}
			//var pinGroup = 0; // 1 = linx_pin_group_propagated constant
			Module._graph_set_propagated_pin(result, i, namePtr, vertexIndex, pinInfo.pinIndex, pinInfo.pinGroup);
			//console.log("compiled propagated pin");
		}
		return result;
	}
}
