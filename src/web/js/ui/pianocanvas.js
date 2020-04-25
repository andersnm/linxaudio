
function PianoCanvas(options) {

	if (!options) {
		return false;
	}

	if (!options.container) {
		return false;
	}

	this.keyTab = [0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0];
	this.keyPos = [0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6];

	this.container = options.container;
	this.keys = options.keys ? options.keys : 48;
	this.onnoteon = (typeof(options.onnoteon) == "function") ? options.onnoteon : null;
	this.onnoteoff = (typeof(options.onnoteoff) == "function") ? options.onnoteoff : null;
	
	this.canvas = document.createElement("canvas");
	this.canvas.width = this.container.offsetWidth;
	this.canvas.height = this.container.offsetHeight;
	this.canvas.classList.add("unselectable");
	this.container.appendChild(this.canvas);

	this.context = this.canvas.getContext("2d");

	this.mouseDownNote = -1;
	var self = this;
	
	window.addEventListener("resize", function() {
		self.redraw();
	});
	
	this.canvas.addEventListener("mousedown", function(e) {	
		var offset = HTMLElementHelper.offset(self.canvas);
		var note = self.hitTest(e.pageX - offset.left, e.pageY - offset.top);
		if (note == -1) {
			return ;
		}
		self.mouseDownNote = note;
		if (self.onnoteon) {
			self.onnoteon(note);
		}
		self.redraw();
	});

	this.canvas.addEventListener("mousemove", function(e) {
		if (self.mouseDownNote == -1) {
			return ;
		}
		var offset = HTMLElementHelper.offset(self.canvas);
		var note = self.hitTest(e.pageX - offset.left, e.pageY - offset.top);
		if (note == -1 || note == self.mouseDownNote) {
			return ;
		}

		if (self.onnoteoff) {
			self.onnoteoff(self.mouseDownNote);
		}		
		self.mouseDownNote = note;
		if (self.onnoteon) {
			self.onnoteon(note);
		}
		self.redraw();
	});
	
	this.canvas.addEventListener("mouseup", function(e) {
		if (self.mouseDownNote == -1) {
			return ;
		}
		if (self.onnoteoff) {
			self.onnoteoff(self.mouseDownNote);
		}
		self.mouseDownNote = -1;
		self.redraw();
	});
	
	setTimeout(function() {
		self.redraw();
	}, 0);
}

PianoCanvas.prototype.detach = function() {
	this.container.removeChild(this.canvas);
}

PianoCanvas.prototype.getWhiteKeys = function() {
	var octaves = Math.floor(this.keys / 12);
	var note = this.keys % 12;
	return octaves * 7 + this.keyPos[note];
}

PianoCanvas.prototype.getNoteRect = function(note) {
	var octave = Math.floor(note / 12);
	var index = this.keyPos[note % 12] + octave * 7;
	var whiteKeys = this.getWhiteKeys();
	var keyWidth = this.canvas.width / whiteKeys;
	if (this.keyTab[note % 12] == 0) {		
		return { 
			x : index * keyWidth,
			y : 0,
			width : keyWidth - 1,
			height : this.canvas.height - 1
		};
	} else if (this.keyTab[note % 12] == 1) {
		return { 
			x : index * keyWidth + keyWidth / 3 * 2,
			y : 0,
			width : keyWidth / 3 * 2 - 1,
			height : this.canvas.height / 2
		};
	}
}

PianoCanvas.prototype.redraw = function() {
	this.canvas.width = this.container.offsetWidth;
	this.canvas.height = this.container.offsetHeight;
	
	this.context.fillStyle = "#ffffff";
	this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);

	for (var i = 0; i < this.keys; i++) {
		var octave = Math.floor(i / 12);
		if (this.keyTab[i % 12] == 0) {
			var rc = this.getNoteRect(i);
			this.context.beginPath();
			this.context.rect(rc.x, rc.y, rc.width, rc.height);
			this.context.stroke();
			
			if (this.mouseDownNote == i) {
				this.context.fillStyle = "#888888";
			} else {
				this.context.fillStyle = "#ffffff";
			}
			this.context.fillRect(rc.x + 1, rc.y + 1, rc.width - 2, rc.height - 2);
		}
	}

	this.context.fillStyle = "#000000";
	for (var i = 0; i < this.keys; i++) {
		var octave = Math.floor(i / 12);
		if (this.keyTab[i % 12] == 1) {
			var rc = this.getNoteRect(i);
			if (this.mouseDownNote == i) {
				this.context.fillStyle = "#888888";
			} else {
				this.context.fillStyle = "#000000";
			}
			this.context.fillRect(rc.x, rc.y, rc.width, rc.height);
		}
	}
}

PianoCanvas.prototype.hitTest = function(hitX, hitY) {
	for (var i = 0; i < this.keys; i++) {
		var octave = Math.floor(i / 12);
		if (this.keyTab[i % 12] == 1) {
			var rc = this.getNoteRect(i);			
			if ((hitX >= rc.x && hitX <= rc.x + rc.width) && (hitY >= rc.y && hitY < rc.height)) {
				return i;
			}
		}
	}

	for (var i = 0; i < this.keys; i++) {
		var octave = Math.floor(i / 12);
		if (this.keyTab[i % 12] == 0) {
			var rc = this.getNoteRect(i);			
			if ((hitX >= rc.x && hitX <= rc.x + rc.width) && (hitY >= rc.y && hitY < rc.height)) {
				return i;
			}
		}
	}
	return -1;
}

