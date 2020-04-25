// Sequence popup = edit or create which column go in a sequence track
// (the event handlers) should automatically hide already-used pins and automatically propagate/unpropagate pins in use
// loading projects w/o a sequence should create one, and ensure its layout matches etc, such that any changes in the propagations are picked up
// i.e syncPattern2.0
// problem med flere pattrns! så lenge vi er top-level - men vi kan skule propagate på top-level og automatiere alt, f.esk create master shortcut bli ENESTE måte å få
// stereo output....... ?

function SequencePopup() {
	var self = this

	function overlayClick(e) {
		if (e.target == self.container) {
			self.hide();
		}
	}

	function overlayKeydown(e) {
		if (e.keyCode == 27) {
			self.hide();
			e.stopPropagation();
		} else if (e.keyCode == 13) {
			save();
			self.hide();
			e.stopPropagation();
		}
	}
	
	function save() {
		var pins = [];
		for (var i = 0; i < self.list.options.length; i++) {
			var opt = self.list.options[i];
			if (!opt.selected) continue;
			
			var pin = self.pins[opt.value];
			pins.push(pin);
		}
		self.emit("setPins", self.sequenceTrackIndex, pins);
	}
	
	this.sequenceTrackIndex = -1;
	this.pins = null;
	
	this.container = document.createElement("overlay");
	this.container.addEventListener("click", overlayClick);
	this.container.addEventListener("keydown", overlayKeydown);
	
	this.popup = document.createElement("popup");
	this.popup.tabIndex = 0;
	this.popup.style.overflow = "auto";
	this.popup.style.padding = "10px";

	// TODO: describe somewhere what this is?
	this.list = document.createElement("select");
	this.list.size = 2;
	this.list.multiple = true;
	this.list.style.position = "absolute";
	this.list.style.left = "0";
	this.list.style.top = "0";
	this.list.style.width = "100%";
	this.list.style.bottom = "0";
	//this.list.addEventListener("dblclick", listDoubleClick);
	//this.list.addEventListener("change", listChange);
	this.popup.appendChild(this.list);

	this.container.appendChild(this.popup);
	
	Events(this);
	
}

SequencePopup.prototype.show = function() {
	this.container.style.display = "block";
	this.popup.focus();
}

SequencePopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}

SequencePopup.prototype.bind = function(sequenceTrackIndex) {
	var self = this;
	this.sequenceTrackIndex = sequenceTrackIndex;
	this.pins = [];
	this.emit("requestPins", sequenceTrackIndex, this.pins);
	
	HTMLSelectHelper.bindArray(this.list, this.pins, bindPin);
	
	function bindPin(opt, pin, index) {
		opt.text = pin.vertex + ": " + pin.pin;
		opt.value = index;
		opt.selected = pin.selected;
	}
}
