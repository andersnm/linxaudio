function ConnectionPopup() {
	var self = this;

	function overlayClick(e) {
		if (e.target == self.container) {
			self.emit("connect", self.targetVertexId, self.sourceVertexId, self.edges);
			self.hide();
		}
	}

	function overlayKeydown(e) {
		if (e.keyCode == 27) {
			self.hide();
			e.stopPropagation();
		} else if (e.keyCode == 13) {
			self.emit("connect", self.targetVertexId, self.sourceVertexId, self.edges);
			self.hide();
			e.stopPropagation();
		}
	}
	
	function targetListChange() {
		// 1. remove all edges with the current source
		// 2. add back new edges from the selected targets to the source
		self.edges = self.edges.filter(function(edge) {
			return edge.source != self.sourceList.value;
		});
		for (var i = 0; i < self.targetList.options.length; i++) {
			var opt = self.targetList.options[i];
			if (opt.selected && opt.value != "") {
				self.edges.push({target:opt.value, source:self.sourceList.value});
			}
		}
		/*
		if (self.targetList.value == "") {
			self.edges = self.edges.filter(function(edge) {
				return edge.source != self.sourceList.value;
			});
		} else {
			// for each option, create edges for all targets! can be >1 q: are edges all pins, or just the current target pin
			self.edges.push({target:self.targetList.value, source:self.sourceList.value});
		}*/
	}
	
	function sourceListChange() {
		self.updateTargetPins();
	}
	
	this.container = document.createElement("overlay");
	this.container.addEventListener("click", overlayClick);
	this.container.addEventListener("keydown", overlayKeydown);
	
	this.popup = document.createElement("popup");
	this.popup.tabIndex = 0;
	
	// lists with pins on source and target modules
	this.sourceLabel = document.createElement("label");
	this.sourceLabel.innerHTML = "Source Module";
	this.sourceLabel.style.position = "absolute";
	this.sourceLabel.style.left = "0";
	this.sourceLabel.style.top = "0";
	this.sourceLabel.style.width = "50%";
	this.sourceLabel.style.height = "24px";
	this.popup.appendChild(this.sourceLabel);
	
	this.sourceList = document.createElement("select");
	this.sourceList.size = 2;
	this.sourceList.style.position = "absolute";
	this.sourceList.style.left = "0";
	this.sourceList.style.top = "24px";
	this.sourceList.style.width = "50%";
	this.sourceList.style.bottom = "0";
	this.sourceList.addEventListener("change", sourceListChange);
	this.popup.appendChild(this.sourceList);

	this.targetLabel = document.createElement("label");
	this.targetLabel.innerHTML = "Target Module";
	this.targetLabel.style.position = "absolute";
	this.targetLabel.style.left = "50%";
	this.targetLabel.style.top =  "0";
	this.targetLabel.style.width = "50%";
	this.targetLabel.style.height = "24px";
	this.popup.appendChild(this.targetLabel);

	this.targetList = document.createElement("select");
	this.targetList.size = 2;
	this.targetList.style.position ="absolute";
	this.targetList.style.left = "50%";
	this.targetList.style.top = "24px";
	this.targetList.style.width = "50%";
	this.targetList.style.bottom = "0";
	this.targetList.multiple = true;
	this.targetList.addEventListener("change", targetListChange);
	this.popup.appendChild(this.targetList);

	this.container.appendChild(this.popup);
	
	Events(this);
}

ConnectionPopup.prototype.show = function() {
	this.container.style.display = "block";
	this.sourceList.focus();
}

ConnectionPopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}

ConnectionPopup.prototype.updateTargetPins = function() {
	var targetPins = [];
	this.emit("requestTargetList", this.targetVertexId, this.sourceVertexId, this.sourceList.value, targetPins);

	console.log("targetPins.length  = " + targetPins.length);
	console.log("edges.length  = " + this.edges.length);
	
	targetPins.splice(0, 0, { name: "<no mapping>", value : "" });
	HTMLSelectHelper.bindNameValueList(this.targetList, targetPins);
	
	var self = this;
	var sourceEdges = this.edges.filter(function(edge) {
		//console.log("source eg: " + edge.source + " vs " + self.sourceList.value);
		return edge.source == self.sourceList.value;
	});
	
	HTMLSelectHelper.clearSelectOptions(this.targetList.options);
	
	if (sourceEdges.length == 0) {
		this.targetList.value = ""; // no mapping
	} else {
		for (var i = 0; i < sourceEdges.length; i++) {
			var edge = sourceEdges[i];		
			var toPin = HTMLSelectHelper.getOptionByValue(this.targetList.options, edge.target);
			toPin.selected = true;		
		}
	}
}

ConnectionPopup.prototype.bindVertices = function(targetVertexId, sourceVertexId, edges) {
	this.sourceLabel.innerHTML = sourceVertexId;
	this.targetLabel.innerHTML = targetVertexId;
	this.edges = edges;
	
	this.sourceVertexId = sourceVertexId;
	this.targetVertexId = targetVertexId;

	var sourcePins = [];
	var targetPins = [];
	this.emit("requestSourceList", sourceVertexId, sourcePins);
	
	HTMLSelectHelper.bindNameValueList(this.sourceList, sourcePins);
	
	if (sourcePins.length > 0) {
		this.sourceList.value = this.sourceList.options[0].value;
		this.updateTargetPins();
	}
}
