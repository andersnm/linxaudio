function ParameterPopup() {
	var self = this;

	function floatingKeydown(e) {
		if (e.keyCode == 27) {
			self.hide();
		}
	}
	
	function nameChange() {
		self.emit("rename", self.graphId, self.vertexId, self.label.value);
		self.vertexId = self.label.value;
	}
	
	function mouseDown(e) {
		self.mouseDown(e);
	}

	function mouseMove(e) {
		self.mouseMove(e);
	}

	function mouseUp(e) {
		self.mouseUp(e);
	}

	this.drag = false;
	this.dragPosition = [0, 0];
	
	this.container = document.createElement("floating");
	this.container.tabIndex = 0;
	this.container.style.width = "300px";
	this.container.addEventListener("keydown", floatingKeydown);
	this.container.addEventListener("mousedown", mouseDown);
	this.container.addEventListener("mousemove", mouseMove);
	this.container.addEventListener("mouseup", mouseUp);
	
	this.label = document.createElement("input");
	this.label.type = "text";
	this.label.style.position = "absolute";
	this.label.style.top = "0";
	this.label.style.left = "0";
	this.label.style.width = "100%";
	this.label.style.height = "24px";
	this.label.value = "Module Name";
	this.label.addEventListener("change", nameChange);
	
	this.sliderContainer = document.createElement("div");
	this.sliderContainer.style.position = "absolute";
	this.sliderContainer.style.top = "24px";
	this.sliderContainer.style.width = "100%";
	this.sliderContainer.style.bottom = "0";
	this.sliderContainer.style.overflow = "auto";
	
	this.container.appendChild(this.label);
	this.container.appendChild(this.sliderContainer);
	
	Events(this);
}

ParameterPopup.prototype.mouseDown = function(e) {
	if (e.target == this.label) {
		this.drag = true;
		this.dragPosition[0] = e.pageX - this.container.offsetLeft;
		this.dragPosition[1] = e.pageY - this.container.offsetTop;
	}
}

ParameterPopup.prototype.mouseMove = function(e) {
	if (this.drag) {
		this.container.style.left = e.pageX - this.dragPosition[0];
		this.container.style.top = e.pageY - this.dragPosition[1];
	}
}

ParameterPopup.prototype.mouseUp = function(e) {
	this.drag = false;
}

ParameterPopup.prototype.bindVertex = function(graphId, vertexId, name, sliders) {
	this.graphId = graphId;
	this.vertexId = vertexId;
	this.label.value = name;
	
	for (var i = 0; i < sliders.length; i++) {
		var slider = sliders[i];
		this.createSlider(slider);//.pin, slider.name, slider.value);
	}
}

ParameterPopup.prototype.getValueDescription = function(pinName, value, callback) {
	// plugin->get_value_description
	// TODO: this is async; send in callback when describe has been obtained
	//var result = [];
	this.emit("describeValue", this.graphId, this.vertexId, pinName, value, callback);
	/*if (result.length > 0) {
		return result[0];
	}
	return value.toString();*/
}

ParameterPopup.prototype.createSlider = function(slider) { //pin, name, value) {

	var row = document.createElement("div");
	row.style.position = "relative";
	row.style.width = "100%";
	row.style.height = "24px";
	
	var nameLabel = document.createElement("label");
	nameLabel.innerHTML = slider.name;
	nameLabel.style.position = "absolute";
	nameLabel.style.overflow = "hidden";
	nameLabel.style.left = "0";
	nameLabel.style.width = "70px";

	var valueLabel = document.createElement("label");
	valueLabel.style.position = "absolute";
	valueLabel.style.overflow = "hidden";
	valueLabel.style.width = "70px";
	valueLabel.style.right = "0";
	//valueLabel.innerHTML = this.getValueDescription(slider.name, slider.value);
	this.getValueDescription(slider.name, slider.value, function(description) {
		valueLabel.innerHTML = description;
	});
	
	var inputContainer = document.createElement("div");
	inputContainer.style.position = "absolute";
	inputContainer.style.left = "70px";
	inputContainer.style.right = "70px";
	inputContainer.style.height = "24px";
	
	var self = this;
	
	if (slider.pinType == "midinote") {
		// MIDI! -> pianocanvas
		var piano = new PianoCanvas({
			container : inputContainer,
			onnoteon : function(note) {
				self.emit("noteon", self.graphId, self.vertexId, slider.name, note);
			},
			onnoteoff : function(note) {
				self.emit("noteoff", self.graphId, self.vertexId, slider.name, note);
			},
			keys:24
		});
	} else {
		var step;
		var range = slider.maxValue - slider.minValue;
		if (slider.pinType == "int" || slider.pinType == "byte" || slider.pinType == "word") {
			// int
			step = 1;
		} else if (slider.pinType == "float") {
			// float
			if (slider.precision == 0) {
				step = 0;
			} else {
				step = slider.precision;
				// TODO: should be divisible by 1/range in firefox, and the default value
			}
		} else {
			console.log("error: unknown pin type " + slider.pinType);
		}
		
		var steps = range / step;
		
			// midi 
		if (steps >= 32000) {
			var sliderInput = document.createElement("input");
			sliderInput.type = "text";
			// default value
			sliderInput.value = slider.value; //pin.defaultValue;
			sliderInput.style.width = "100%";
			sliderInput.style.height = "100%";
			sliderInput.addEventListener("change", function(e) {
				var value = parseFloat(this.value);
				//valueLabel.innerHTML = self.getValueDescription(slider.name, value);
				self.getValueDescription(slider.name, value, function(description) {
					valueLabel.innerHTML = description;
				});
				self.emit("change", self.graphId, self.vertexId, slider.name, value);
			});
			inputContainer.appendChild(sliderInput);
		} else {
			var sliderInput = document.createElement("input");
			sliderInput.type = "range";
			sliderInput.min = slider.minValue;
			sliderInput.max = slider.maxValue;
			sliderInput.step = step;
			sliderInput.value = slider.value; //pin.defaultValue;
			sliderInput.addEventListener("input", function(e) {
				var value = parseFloat(this.value);
				//valueLabel.innerHTML = self.getValueDescription(slider.name, value);
				//valueLabel.innerHTML = this.value;
				self.getValueDescription(slider.name, value, function(description) {
					valueLabel.innerHTML = description;
				});
			});
			sliderInput.addEventListener("change", function(e) {
				var value = parseFloat(this.value);			
				//valueLabel.innerHTML = value;
				//valueLabel.innerHTML = self.getValueDescription(slider.name, value);
				self.getValueDescription(slider.name, value, function(description) {
					valueLabel.innerHTML = description;
				});
				self.emit("change", self.graphId, self.vertexId, slider.name, value);
			});
			sliderInput.style.width = "100%";
			sliderInput.style.height = "100%";
			sliderInput.style.margin = "0";
			sliderInput.style.padding = "0";
			inputContainer.appendChild(sliderInput);
		}
	}
	

	row.appendChild(nameLabel);
	row.appendChild(inputContainer);
	row.appendChild(valueLabel);

	this.sliderContainer.appendChild(row);
}

ParameterPopup.prototype.show = function() {
	this.container.style.display = "block";
	this.label.focus();
}

ParameterPopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}
