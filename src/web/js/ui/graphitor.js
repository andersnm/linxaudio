/*

MV Master		C6BEAA
MV Generator		A9AEC7
MV Effect		C7ADA9
MV Background		DAD6C9
MV Machine Text		000000

*/

var VERTEX_WIDTH = 0.2;
var VERTEX_HEIGHT = 0.15;

var DRAGTYPE_NONE = -1;
var DRAGTYPE_MOVE = 0;
var DRAGTYPE_CONNECT = 1;
var DRAGTYPE_SELECT = 2;

function GraphitorVertexStyle(textColor, fillColor, borderColor) {
	this.textColor = textColor || "#000000";
	this.fillColor = fillColor || "#ffffff";
	this.borderColor = borderColor || "#000000";
}

function GraphitorVertex(id, name, x, y, style) {
	this.id = id;
	this.name = name;
	this.x = x;
	this.y = y;
	this.style = style || new GraphitorVertexStyle();
}

GraphitorVertex.prototype.setPosition = function(x, y) {
	this.x = x;
	this.y = y;
}

function GraphitorEdge(targetVertex, sourceVertex) {
	this.targetVertex = targetVertex;
	this.sourceVertex = sourceVertex;
}

function Graphitor(mainContainer, options) {

	var self = this;
	
	function windowResize() {
		self.canvas.width = mainContainer.offsetWidth; //window.innerWidth;
		self.canvas.height = mainContainer.offsetHeight - TOOLBAR_HEIGHT; //window.innerHeight;
		self.dirty = true;
	}

	var defaultOptions = { edgeDigest : false };

	this.options = [];
	for (var defaultOption in defaultOptions) {
		if (options && options.hasOwnProperty(defaultOption)) {
			this.options[defaultOption] = options[defaultOption];
		} else {
			this.options[defaultOption] = defaultOptions[defaultOption];
		}
	}
	
	this.vertices = [];
	this.edges = [];
	this.selectedVertices = [];
	this.clickPosition = { x : 0, y : 0 };
	this.drag = false;
	this.dragType = DRAGTYPE_NONE;
	this.dragPosition = null;
	this.dragConnectSourceVertex = null;
	this.dragConnectTargetPosition = { x : 0, y : 0 };
	this.dirty = true;

	this.container = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.container, "absolute", "0", "0", "0", "0");

	this.initToolbar();
	this.initEditor();

	mainContainer.appendChild(this.container);
	window.addEventListener("resize", windowResize);
	windowResize();
	
	Events(this);
}

Graphitor.prototype.initToolbar = function() {

	var self = this;
	
	function insertClick() { self.emit("charKeydown", ""); }
	function cutClick() { self.emit("cut"); }
	function copyClick() { self.emit("copy"); }
	function pasteClick() { self.emit("paste"); }
	function utilityClick() { self.emit("utility"); }

	this.toolbarContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(this.toolbarContainer, "absolute", "0", "0", "100%", TOOLBAR_HEIGHT + "px");
	this.toolbarContainer.style.backgroundColor = "#dddddd";

	this.graphToolbar = new ViewToolbar(this.toolbarContainer, "small");
	this.graphToolbar.insertButton = this.graphToolbar.createButton("Insert Plugin...", "mac", insertClick);
	this.graphToolbar.cutButton = this.graphToolbar.createButton("Cut", "cut", cutClick);
	this.graphToolbar.copyButton = this.graphToolbar.createButton("Copy", "cpy", copyClick);
	this.graphToolbar.pasteButton = this.graphToolbar.createButton("Paste", "pas", pasteClick);

	this.graphToolbar.utilityButton = this.graphToolbar.createButton("Utility...", "utl", utilityClick);

	this.graphToolbar.pathLabel = this.graphToolbar.createButton(" ");
	this.graphToolbar.pathLabel.style.cssFloat = "right";

	this.container.appendChild(this.toolbarContainer);
}

Graphitor.prototype.updatePath = function() {
	var result = { value : "" };
	this.emit("requestGraphPath", result);
	this.graphToolbar.pathLabel.firstChild.innerHTML = result.value;
}

Graphitor.prototype.initEditor = function() {
	var self = this;
	
	function editorKeyDown(e) {
		self.editorKeyDown(e);
	}

	function editorKeyPress(e) {
		self.editorKeyPress(e);
	}

	function editorMouseDown(e) {
		self.editorMouseDown(e);
	}

	function editorMouseMove(e) {
		self.editorMouseMove(e);
	}

	function editorMouseUp(e) {
		self.editorMouseUp(e);
	}

	function editorMouseDblClick(e) {
		self.editorMouseDblClick(e);
	}

	this.canvas = document.createElement("canvas");
	this.context = this.canvas.getContext("2d");

	this.canvas.tabIndex = "0"; // means it holds focus when clicked
	this.canvas.classList.add("unselectable");
	this.canvas.addEventListener("keydown", editorKeyDown);
	this.canvas.addEventListener("keypress", editorKeyPress);
	this.canvas.addEventListener("mousedown", editorMouseDown);
	this.canvas.addEventListener("mousemove", editorMouseMove);
	this.canvas.addEventListener("mouseup", editorMouseUp);
	this.canvas.addEventListener("dblclick", editorMouseDblClick);

	this.canvasContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.canvasContainer, "absolute", "0", "24px", "0", "0");
	this.canvasContainer.appendChild(this.canvas);
	
/*	this.canvas.width = mainContainer.offsetWidth; //window.innerWidth;
	this.canvas.height = mainContainer.offsetHeight; //window.innerHeight;*/
	this.container.appendChild(this.canvasContainer);

}


Graphitor.prototype.show = function() {
	this.canvas.style.display = "block";
	this.canvas.focus();
}

Graphitor.prototype.hide = function() {
	this.canvas.style.display = "none";
}

Graphitor.prototype.addVertex = function(id, name, x, y, style) {
	var node = new GraphitorVertex(id, name, x, y, style);
	this.vertices.push(node);
	this.dirty = true;
}

Graphitor.prototype.addEdge = function(targetId, sourceId) {
	var targetVertex = this.getVertexById(targetId);
	var sourceVertex = this.getVertexById(sourceId);
	var edge = new GraphitorEdge(targetVertex, sourceVertex);
	this.edges.push(edge);
	this.dirty = true;
}

Graphitor.prototype.getVertexById = function(id) {
	for (var i = 0; i < this.vertices.length; i++) {
		var vertex = this.vertices[i];
		if (vertex.id == id) {
			return vertex;
		}
	}
}

function distToSegment(p, v, w) { 
	// Return minimum distance between line segment vw and point p
	// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment

	function sqr(x) { 
		return x * x;
	}

	function dist2(v, w) { 
		return sqr(v.x - w.x) + sqr(v.y - w.y);
	}

	function distToSegmentSquared(p, v, w) {
		var l2 = dist2(v, w);
		if (l2 == 0) return dist2(p, v);
		var t = ((p.x - v.x) * (w.x - v.x) + (p.y - v.y) * (w.y - v.y)) / l2;
		if (t < 0) return dist2(p, v);
		if (t > 1) return dist2(p, w);
		return dist2(p, { x: v.x + t * (w.x - v.x), y: v.y + t * (w.y - v.y) });
	}

	return Math.sqrt(distToSegmentSquared(p, v, w)); 
}

Graphitor.prototype.getEdgeByPosition = function(x, y) {
	var p = { x : x, y : y };
	
	for (var i = 0; i < this.edges.length; i++) {
		var edge = this.edges[i];
		var v = edge.sourceVertex;
		var w = edge.targetVertex;
		
		// center of line
		var cx = (v.x + w.x) / 2;
		var cy = (v.y + w.y) / 2;
		
		var dx = (cx - x);
		var dy = (cy - y);
		var dist  = Math.sqrt(dx * dx + dy * dy);
		
		//var dist = distToSegment(p, v, w);
		if (dist < 0.03) {
			return edge;
		}
	}
}

Graphitor.prototype.getVertexByPosition = function(x, y) {
	x -= VERTEX_WIDTH / 2;
	y -= VERTEX_HEIGHT / 2;

	for (var i = 0; i < this.vertices.length; i++) {
		var vertex = this.vertices[i];
		if (vertex.x >= x && vertex.x < x + VERTEX_WIDTH && vertex.y >= y && vertex.y < y + VERTEX_HEIGHT) {
			return vertex;
		}
	}
	return null;
}

Graphitor.prototype.getEdgesByVertex = function(vertex) {
	return this.edges.filter(function(edge) {
		return edge.sourceVertex == vertex || edge.targetVertex == vertex;
	});
}

Graphitor.prototype.getEdgesByVertices = function(vertices) {
	var self = this;
	var result = [];
	vertices.forEach(function(vertex) {
		var edges = self.getEdgesByVertex(vertex);
		edges.forEach(function(edge) {
			if (result.indexOf(edge) == -1) {
				result.push(edge);
			}
		});
	});
	return result;
}

Graphitor.prototype.drawTriangle = function(targetVertex, sourceVertex) {

	var scale = 1;
	var lx = sourceVertex.x - targetVertex.x;
	var ly = sourceVertex.y - targetVertex.y;
	
	var len = Math.sqrt(lx * lx + ly * ly);
	var ux = lx /  len;
	var uy = ly /  len;
	
	var rx = 0*ux - 1*uy;
	var ry = 1*ux + 0*uy;

	var hx = targetVertex.x + lx / 2 + ux * 8 * scale;
	var hy = targetVertex.y + ly / 2 + uy * 8 * scale;

	var fx = (hx + rx * -10 * scale);
	var fy = (hy + ry * -10 * scale);
	
	this.context.beginPath();
	this.context.moveTo(fx, fy);
	this.context.lineTo(hx + rx * -10 * scale, hy + ry * -10 * scale);
	this.context.lineTo(hx + rx *  10 * scale, hy + ry *  10 * scale);
	this.context.lineTo(hx + ux * -16 * scale, hy + uy * -16 * scale);
	this.context.lineTo(fx, fy);

	this.context.fillStyle = "#ffffff";
	this.context.fill();
	this.context.stroke();
	this.context.closePath();
	
}

Graphitor.prototype.updateEdgeDigestBuffer = function() {
	// request edge digest async w callback parameter, the caller calls the callback with the digest after its been been obtained
	var self = this;
	this.emit("requestEdgeDigest", function(buffer) {
		self.edgeDigestBuffer = buffer;
	});
}

Graphitor.prototype.renderEdgeMeters = function() {
	var self = this;

	function transformToScreen(pt) {
		return {
			x : (pt.x + 1) / 2 * self.canvas.width,
			y : (pt.y + 1) / 2 * self.canvas.height
		};
	}

	if (!this.options.edgeDigest) {
		return;
	}
	
	// render current buffer, update async elsewhere
	
	if (this.edgeDigestBuffer) {		
		var width = (VERTEX_WIDTH * self.canvas.width / 2) - 2;
		var height = (VERTEX_HEIGHT * self.canvas.height / 2 / 3);
		var halfHeight = height / 2;

		this.vertices.forEach(function(vertex, vertexIndex) {

			var edges = self.edges.filter(function(edge) {
				return edge.sourceVertex == vertex;
			});

			if (edges.length == 0) {
				self.context.fillStyle = "#666666";
			} else {
				self.context.fillStyle = "#000000";
			}

			var sourcePoint = transformToScreen(vertex);
			
			self.context.fillRect(sourcePoint.x - (width/2), sourcePoint.y + height/2 - 1, width, height);

			var boxWidth = width / edges.length;

			edges.forEach(function(edge, vertexEdgeIndex) {
			
				var edgeIndex = self.edges.indexOf(edge);

				var x = sourcePoint.x - width / 2 + (boxWidth * vertexEdgeIndex);
				var y = sourcePoint.y + height / 2 - 1/* - height / 2*/;

				self.context.beginPath();
				self.context.strokeStyle = "#ffffff";

				for (var i = 0; i < 32; i++) {				
					var value = Math.min(Math.max(self.edgeDigestBuffer[edgeIndex * 32 + i], -0.9), 0.9);
					if (i == 0) {
						self.context.moveTo(x, Math.round(y + halfHeight + value * halfHeight) + 0.5);
					} else {
						self.context.lineTo(x, Math.round(y + halfHeight + value * halfHeight) + 0.5);
					}
					x += (boxWidth / 31);
				}

				self.context.stroke();
				self.context.closePath();
			
			});
			
		});

	}
}

Graphitor.prototype.render = function() {
	var self = this;

	this.updateEdgeDigestBuffer();

	if (!this.dirty) {
		this.renderEdgeMeters();
		return false;
	}
	
	function transformToScreen(pt) {
		return {
			x : (pt.x + 1) / 2 * self.canvas.width,
			y : (pt.y + 1) / 2 * self.canvas.height
		};
	}

	this.context.fillStyle = "#DAD6C9";
	this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
	
	this.context.fillStyle = "#000000";
	this.context.font = "15px Helvetica";
	
	//this.context.textAlign = "right";
	//var path = this.getGraphPath();
	//this.context.fillText(path, this.canvas.width, 15);
	
	if (this.vertices.length == 0) {
		this.context.textAlign = "center";
		this.context.fillText("(empty project - start typing to insert a plugin)", this.canvas.width/2, this.canvas.height/2);
	}
	
	
	this.edges.forEach(function(edge) {
		var sourcePoint = transformToScreen(edge.sourceVertex);
		var targetPoint = transformToScreen(edge.targetVertex);
		self.context.beginPath();
		self.context.strokeStyle = "#000000";
		self.context.moveTo(sourcePoint.x, sourcePoint.y);
		self.context.lineTo(targetPoint.x, targetPoint.y);
		self.context.stroke();
		self.context.closePath();

		self.drawTriangle(targetPoint, sourcePoint);
	});

	if (this.dragConnectSourceVertex) {
		this.context.beginPath();
		var sourcePoint = transformToScreen(this.dragConnectSourceVertex);
		var targetPoint = transformToScreen(this.dragConnectTargetPosition);
		this.context.strokeStyle = "#000000";
		this.context.moveTo(sourcePoint.x, sourcePoint.y);
		this.context.lineTo(targetPoint.x, targetPoint.y);
		this.context.stroke();
		this.context.closePath();
	}

	this.context.beginPath();
	this.vertices.forEach(function(vertex) {
		var x = vertex.x - VERTEX_WIDTH / 2;
		var y = vertex.y - VERTEX_HEIGHT / 2;
		
		var vertexPoint = transformToScreen(vertex);
		var width = VERTEX_WIDTH * self.canvas.width / 2;
		var height = VERTEX_HEIGHT * self.canvas.height / 2;
		
		self.context.fillStyle = vertex.style.fillColor;//"#A9AEC7";
		self.context.fillRect(vertexPoint.x - width / 2, vertexPoint.y - height / 2, width, height);

		self.context.strokeStyle = vertex.style.borderColor;
		self.context.strokeRect(vertexPoint.x - width / 2, vertexPoint.y - height / 2, width, height);
		
		self.context.fillStyle = vertex.style.textColor;//"#000000";
		self.context.textAlign = "center";
		self.context.font = "15px Helvetica";
		if (self.options.edgeDigest) {
			self.context.fillText(vertex.name, vertexPoint.x, vertexPoint.y + 6 - 9);//(height/3));
		} else {
			self.context.fillText(vertex.name, vertexPoint.x, vertexPoint.y + 6);//(height/3));
		}
		if (self.selectedVertices.indexOf(vertex) != -1) {
			
			self.context.strokeRect(vertexPoint.x - width / 2 - 4, vertexPoint.y - height / 2 - 4, width + 8, height + 8);
		}
	});
	this.context.stroke();
	this.context.closePath();

	this.renderEdgeMeters();
	
	if (this.dragType == DRAGTYPE_SELECT) {
		var p1 = transformToScreen(this.dragPosition);
		var p2 = transformToScreen(this.dragPreviousPosition);
		var selectWidth = p2.x - p1.x;
		var selectHeight = p2.y - p1.y;
		this.context.strokeRect(p1.x, p1.y, selectWidth, selectHeight);
		this.context.closePath();
	}

	this.dirty = false;
}

Graphitor.prototype.editorKeyDown = function(e) {

	switch (e.keyCode) {
	
		case 13:
			// ascend into subgraph
			this.emit("enterSubgraph");
			this.updatePath();
			break;
		case 27:
			// descend out of subgraph
			this.emit("leaveSubgraph");
			this.updatePath();
			break;
		case 46:
			// delete
			this.emit("deleteVertices", this.selectedVertices);
			break;
		case 67: // 'c'
			if (e.ctrlKey) {
				this.emit("copy");
			}
			break;
		case 88: // 'x'
			if (e.ctrlKey) {
				this.emit("cut");
			}
			break;
		case 86: // 'v'
			if (e.ctrlKey) {
				this.emit("paste");
			}
			break;
	}
}

Graphitor.prototype.editorKeyPress = function(e) {
	if (e.ctrlKey || e.altKey || e.charCode == 0) {
		return ;
	}

	var keyString = String.fromCharCode(e.charCode);
	if (/[a-zA-Z0-9-_ ]/.test(keyString)) {
		// if alphanumeric, emit event we want to search for a new module starting with this keycode
		this.emit("charKeydown", keyString);
		e.preventDefault();
	}
}

Graphitor.prototype.editorMouseDown = function(e) {
	var offset = HTMLElementHelper.offset(this.canvas);
	var x = (e.pageX - offset.left) / this.canvas.width * 2 - 1;
	var y = (e.pageY - offset.top) / this.canvas.height * 2 - 1;
	
	this.clickPosition.x = x;
	this.clickPosition.y = y;

	this.dirty = true;

	var vertex = this.getVertexByPosition(x, y);
	var edge = this.getEdgeByPosition(x, y);

	if (!vertex && !edge) {
		this.dragType = DRAGTYPE_SELECT;
		this.dragPosition = { x : x, y : y };
		this.dragPreviousPosition = { x : x, y : y };
		this.selectedVertices = [ ];
		this.drag = true;
	} else if (vertex) {
		if (e.shiftKey) {
			this.dragType = DRAGTYPE_CONNECT;
			this.dragConnectTargetPosition.x = x;
			this.dragConnectTargetPosition.y = y;
			this.dragConnectSourceVertex = vertex;
		} else {
			if (this.selectedVertices.indexOf(vertex) == -1) {
				this.selectedVertices = [ vertex ];
			}

			this.dragType = DRAGTYPE_MOVE;
			this.dragPosition = { x : x, y : y };
			this.dragPreviousPosition = { x : x, y : y };
		}
		this.drag = true;
	}
}

Graphitor.prototype.editorMouseMove = function(e) {
	if (!this.drag) {
		return ;
	}
	
	var offset = HTMLElementHelper.offset(this.canvas);
	var x = (e.pageX - offset.left) / this.canvas.width * 2 - 1;
	var y = (e.pageY - offset.top) / this.canvas.height * 2 - 1;
	//var x = ( e.clientX / this.canvas.width ) * 2 - 1;
	//var y = ( e.clientY / this.canvas.height ) * 2 - 1;

	if (this.dragType == DRAGTYPE_SELECT) {
		this.dragPosition = { x : x, y : y };
		this.dirty = true;
	} else if (this.dragType == DRAGTYPE_CONNECT) {	
		this.dragConnectTargetPosition.x = x;
		this.dragConnectTargetPosition.y = y;
	} else if (this.dragType == DRAGTYPE_MOVE) {
		var dx = x - this.dragPreviousPosition.x;
		var dy = y - this.dragPreviousPosition.y;
		for (var i = 0; i < this.selectedVertices.length; i++) {
			var vertex = this.selectedVertices[i];
			vertex.setPosition(vertex.x + dx, vertex.y + dy);
		}

		this.dragPreviousPosition = { x : x, y : y };
	}
	this.dirty = true;
}

Graphitor.prototype.editorMouseUp = function(e) {
	var self = this;
	var offset = HTMLElementHelper.offset(this.canvas);
	var x = (e.pageX - offset.left) / this.canvas.width * 2 - 1;
	var y = (e.pageY - offset.top) / this.canvas.height * 2 - 1;

	if (!this.drag) {
		// for edge click:
		// http://stackoverflow.com/questions/849211/shortest-distance-between-a-point-and-a-line-segment
		var edge = this.getEdgeByPosition(x, y);
		if (edge) {
			this.emit("editConnection", edge.targetVertex, edge.sourceVertex);
			return ;
		}

		return;
	}

	if (this.dragType == DRAGTYPE_SELECT) {
		// add plugins in rect to selectedVertices
		this.vertices.forEach(function(vertex) {
			var p1 = { 
				x : Math.min(self.dragPosition.x, self.dragPreviousPosition.x),
				y : Math.min(self.dragPosition.y, self.dragPreviousPosition.y)
			};
			var p2 = { 
				x : Math.max(self.dragPosition.x, self.dragPreviousPosition.x),
				y : Math.max(self.dragPosition.y, self.dragPreviousPosition.y)
			};
			if (vertex.x >= p1.x && vertex.x < p2.x && vertex.y >= p1.y && vertex.y < p2.y) {
				self.selectedVertices.push(vertex);
			}
		});
	} else if (this.dragType == DRAGTYPE_CONNECT) {
		var targetVertex = this.getVertexByPosition(x, y);
		if (targetVertex && targetVertex != this.dragConnectSourceVertex) {
			this.emit("editConnection", targetVertex, this.dragConnectSourceVertex);
		}		
		this.dragConnectSourceVertex = null;
	} else if (this.dragType == DRAGTYPE_MOVE) {
		this.emit("move", this.selectedVertices);
	}

	this.drag = false;
	this.dirty = true;
	this.dragType = DRAGTYPE_NONE;
}

Graphitor.prototype.editorMouseDblClick = function(e) {
	var offset = HTMLElementHelper.offset(this.canvas);
	var x = (e.pageX - offset.left) / this.canvas.width * 2 - 1;
	var y = (e.pageY - offset.top) / this.canvas.height * 2 - 1;
	//var x = ( e.clientX / this.canvas.width ) * 2 - 1;
	//var y = ( e.clientY / this.canvas.height ) * 2 - 1;

	var vertex = this.getVertexByPosition(x, y);
/*	if (!vertex) {
		return;
	}
*/
	this.emit("doubleClick", vertex);
}

Graphitor.prototype.clear = function() {
	var self = this;
	this.edges = [];
	this.vertices = [];	
	this.selectedVertices = [];
	this.dirty = true;
}
