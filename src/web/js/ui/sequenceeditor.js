var TOOLBAR_HEIGHT = 24;

function SequenceEditor(mainContainer, project) {

	this.project = project;
	this.skipRows = 16;
	this.stepSize = 1;
	this.cursorColumn = 0;
	this.cursorScreenRow = 0;
	this.fontHeight = 16;
	this.dirty = true;

	this.container = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.container, "absolute", "0", "0", "0", "0");

	this.initToolbar();
	this.initPatternList();
	this.initEditor();

	mainContainer.appendChild(this.container);
	
	Events(this);
}

SequenceEditor.prototype.initToolbar = function() {

	var self = this;

	function skipRowsChange(e) {
		self.skipRows = parseInt(e.target.value);
		self.updateCanvas();
	}
	
	function newTrack() {
		self.emit("newTrack");
	}
	
	function editTrack() {
		self.emit("editTrack", self.cursorColumn);
	}

	function bindArrayValue(opt, item) {
		opt.text = item;
		opt.value = item;
	}

	this.toolbarContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(this.toolbarContainer, "absolute", "0", "0", "100%", TOOLBAR_HEIGHT + "px");
	this.toolbarContainer.style.backgroundColor = "#dddddd";

	this.sequenceToolbar = new ViewToolbar(this.toolbarContainer, "small"); 
	this.sequenceToolbar.insertButton = this.sequenceToolbar.createButton("New Track...", "mac", newTrack); 	
	this.sequenceToolbar.editButton = this.sequenceToolbar.createButton("Edit Track...", "mac", editTrack);

	var skipList = [];
	for (var i = 0; i < 64; i++) {
		skipList.push(i + 1);
	}	
	this.sequenceToolbar.skipRowsList = this.sequenceToolbar.createDropdown("Skip rows", this.skipRows, skipList, bindArrayValue, skipRowsChange);
	this.container.appendChild(this.toolbarContainer);
}

SequenceEditor.prototype.initPatternList = function() {
	this.patternListContainer = document.createElement("div");
	HTMLBoxHelper.setWidthTopRightBottom(this.patternListContainer, "absolute", "150px", TOOLBAR_HEIGHT + "px", "0px", "0");
	
	this.patternList = document.createElement("select");
	this.patternList.size = 2;
	this.patternList.style.width = "100%",
	this.patternList.style.height = "100%",
	this.patternListContainer.appendChild(this.patternList);
	
	this.container.appendChild(this.patternListContainer);
}

SequenceEditor.prototype.updatePatternList = function() {
	var patternItems = [];
	if (this.project.sequenceTracks.length > 0) {
		patternItems = this.project.sequenceTracks[this.cursorColumn].patterns.map(function(pattern, index) {
			return { name: index + ". " + pattern.name, value: index};
		});
	}
	HTMLSelectHelper.bindNameValueList(this.patternList, patternItems);
}

SequenceEditor.prototype.gotoCursorPattern = function() {
	var sequenceTrack = this.project.sequenceTracks[this.cursorColumn];
	var sequenceEvent = sequenceTrack.getEventAt(this.cursorScreenRow * this.skipRows);
	if (sequenceEvent) {
		this.emit("gotoPattern", this.cursorColumn, sequenceEvent.patternIndex);
	}
}

SequenceEditor.prototype.show = function() {
	this.container.style.display = "block";
	this.updateCanvas();
	this.editorContainer.focus();
}

SequenceEditor.prototype.hide = function() {
	this.container.style.display = "none";
}

SequenceEditor.prototype.initEditor = function() {
	var self = this;

	function containerScroll(e) {
		self.rowNumberContainer.scrollTop = self.canvasContainer.scrollTop;
		self.headerContainer.scrollLeft = self.canvasContainer.scrollLeft;
		e.preventDefault();
	}

	function editorKeyDown(e) {
		if (!self.project.sequenceTracks.length) {
			return ;
		}
		
		switch (e.keyCode) {
			case 40: self.moveCursor(0, 1); return ;
			case 38: self.moveCursor(0, -1); return;
			case 37: self.moveCursor(-1, 0); return ;
			case 39: self.moveCursor(1, 0); return ;
			case 33: self.moveCursor(0, -16); return ;
			case 34: self.moveCursor(0, 16); return ;
			case 13: self.gotoCursorPattern();
				e.preventDefault();
				return ;
			case 45: // insert
				var sequenceTrack = self.project.sequenceTracks[self.cursorColumn];
				sequenceTrack.shiftSequence(self.cursorScreenRow * self.skipRows, self.skipRows);
				self.dirty = true;
				break;
			case 46: // delete
				var sequenceTrack = self.project.sequenceTracks[self.cursorColumn];
				sequenceTrack.setEventAt(self.cursorScreenRow * self.skipRows, null);
				sequenceTrack.shiftSequence(self.cursorScreenRow * self.skipRows, -self.skipRows);
				self.dirty = true;
				break;
		}
	}

	function editorKeyPress(e) {
		if (e.charCode >= 48 && e.charCode <= 57) {
			// 0..9
			var patternIndex = e.charCode - 48;

			var sequenceTrack = self.project.sequenceTracks[self.cursorColumn];
			if (patternIndex < sequenceTrack.patterns.length) {
				sequenceTrack.setEventAt(self.cursorScreenRow * self.skipRows, patternIndex);
				self.moveCursor(0, 1);
				self.dirty = true;
			}
		}
	}
	
	this.editorContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.editorContainer, "absolute", "0px", TOOLBAR_HEIGHT + "px", "150px", "0");
	this.editorContainer.style.backgroundColor = "#DAD6C9";	
	this.editorContainer.tabIndex = "0";
	this.editorContainer.addEventListener("keydown", editorKeyDown);
	this.editorContainer.addEventListener("keypress", editorKeyPress);

	this.headerContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(this.headerContainer, "absolute", "0", "0px", "100%", "23px");
	this.headerContainer.style.backgroundColor = "#DAD6C9";
	this.headerContainer.style.borderBottom = "1px solid black";
	this.headerContainer.style.overflow = "hidden";

	this.rowNumberContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTopWidthBottom(this.rowNumberContainer, "absolute", "0", "24px", "63px", "0");
	this.rowNumberContainer.style.overflow = "hidden";
	this.rowNumberContainer.style.backgroundColor = "#DAD6C9";
	this.rowNumberContainer.style.textAlign = "right";
	this.rowNumberContainer.style.borderRight = "1px solid black";

	this.editorContainer.appendChild(this.headerContainer);
	this.editorContainer.appendChild(this.rowNumberContainer);

	this.canvasContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.canvasContainer, "absolute", "64px", "24px", "0", "0");
	this.canvasContainer.style.overflow = "auto";
	this.canvasContainer.addEventListener("scroll", containerScroll);

	this.canvas = document.createElement("canvas");
	this.canvas.classList.add("unselectable");
//	this.canvas.tabIndex = "0"; // means it holds focus when clicked
//	this.canvas.addEventListener("keydown", canvasKeyDown);
	//this.canvas.addEventListener("keypress", canvasKeyDown);
	this.context = this.canvas.getContext("2d");
	this.canvasContainer.appendChild(this.canvas);

	this.editorContainer.appendChild(this.canvasContainer);
	this.container.appendChild(this.editorContainer);
}

SequenceEditor.prototype.updateCanvas = function() {
	this.canvas.width = this.project.sequenceTracks.length * 64;
	this.canvas.height = this.project.sequenceLength / this.skipRows * this.fontHeight;
	
	this.rowNumberContainer.innerHTML = "";
	for (var i = 0 ; i < this.project.sequenceLength / this.skipRows; i++) {
		var rowNumber = document.createElement("div");
		rowNumber.style.height = this.fontHeight + "px";
		rowNumber.style.paddingRight = "2px";
		rowNumber.innerHTML = (i * this.skipRows).toString();
		this.rowNumberContainer.appendChild(rowNumber);
	}
	
	this.headerContainer.style.width = "100%";
	this.headerContainer.innerHTML = "";
	
	var self = this;
	this.project.sequenceTracks.forEach(function(sequenceTrack, index) {
		var x = (index + 1) * 64;
		var trackHeader = document.createElement("div");
		HTMLBoxHelper.setLeftTop(trackHeader, "absolute", x + "px", "0px", "63px", "24px");
		trackHeader.style.overflow = "hidden";
		trackHeader.style.borderRight = "1px solid black";
		trackHeader.innerHTML = "Track " + (index + 1).toString();//sequenceTrack.vertex.name;
		self.headerContainer.appendChild(trackHeader);
		
	});
	
	this.dirty = true;
}

SequenceEditor.prototype.moveCursor = function(x, y) {
	this.renderCursor();
	this.cursorColumn += x;
	if (this.cursorColumn < 0) {
		this.cursorColumn = 0;
	}
	if (this.cursorColumn >= this.project.sequenceTracks.length) {
		this.cursorColumn = this.project.sequenceTracks.length - 1;
	}
	
	this.cursorScreenRow += y;
	if (this.cursorScreenRow < 0) {
		this.cursorScreenRow = 0;
	}
	
	var totalScreenRows = Math.floor(this.project.sequenceLength / this.skipRows);
	if (this.cursorScreenRow >= totalScreenRows) {
		this.cursorScreenRow = totalScreenRows - 1;
	}

	this.renderCursor();
	this.scrollToView();
	
	if (x != 0) {
		this.updatePatternList();
	}
}

SequenceEditor.prototype.scrollToView = function() {

	var cursorRow = this.cursorScreenRow;
	var scrollRow = Math.floor(this.canvasContainer.scrollTop / this.fontHeight);
	var screenRows = (Math.floor(this.canvasContainer.offsetHeight / this.fontHeight) - 1);
	
	//console.log(cursorRow + ", " + scrollRow + ", " + screenRows);
	
	if (cursorRow > screenRows + scrollRow) {
		this.canvasContainer.scrollTop = (cursorRow - screenRows) * this.fontHeight;
	} else if (cursorRow < scrollRow) {
		this.canvasContainer.scrollTop = cursorRow * this.fontHeight;
	}

}

SequenceEditor.prototype.renderTrack = function(x, sequenceTrack) {
	
	this.context.lineWidth = 0;
	this.context.beginPath();
	this.context.moveTo(x + 63, 0);
	this.context.lineTo(x + 63, this.canvas.height);
	//this.context.Path();
	this.context.stroke();
	
	var totalScreenRows = Math.floor(this.project.sequenceLength / this.skipRows);
	this.context.fillStyle = "#000000";
	this.context.font = "11px helvetica";
	for (var i = 0; i < totalScreenRows; i++) {
		var sequenceEvent = sequenceTrack.getEventAt(i * this.skipRows);
		if (sequenceEvent) {
			var pattern = sequenceTrack.patterns[sequenceEvent.patternIndex];
			this.context.fillText(pattern.name, x, 11 + i * this.fontHeight);
		} else {
			this.context.fillText("...", x, 11 + i * this.fontHeight);
		}
	}
	
	//console.log("track");
}

SequenceEditor.prototype.renderCursor = function() {
	this.context.globalCompositeOperation = "difference";
	this.context.fillStyle = "#ffffff";
	this.context.fillRect(this.cursorColumn * 64, this.cursorScreenRow * this.fontHeight, 63, this.fontHeight);
	this.context.globalCompositeOperation = "source-over";
}

SequenceEditor.prototype.renderSongPosition = function() {

	var y = this.songPosition * this.fontHeight / this.skipRows;

	this.context.globalCompositeOperation = "difference";
	this.context.fillStyle = "#ffffff";
	this.context.fillRect(0, y, this.canvas.width, 2);
	this.context.globalCompositeOperation = "source-over";
}

SequenceEditor.prototype.render = function() {
	var self = this;

	if (!this.dirty) {
		this.renderSongPosition();
		this.songPosition = this.project.currentTick;
		this.renderSongPosition();		
		return ;
	}

	this.context.fillStyle = "#DAD6C9";
	this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
	
	this.project.sequenceTracks.forEach(function(sequenceTrack, index) {
		self.renderTrack(index * 64, sequenceTrack);
	});
	
	this.songPosition = this.project.currentTick;
	this.renderCursor();
	this.renderSongPosition();
	
	this.dirty = false;
}

SequenceEditor.prototype.keyPress = function(e) {
	if (e.charCode >= 48 && e.charCode <= 57) {
		// 0..9
		var patternIndex = e.charCode - 48;

		var sequenceTrack = this.project.sequenceTracks[this.cursorColumn];
		if (patternIndex < sequenceTrack.patterns.length) {
			sequenceTrack.setEventAt(this.cursorScreenRow * this.skipRows, patternIndex);
			this.moveCursor(0, 1);
			this.dirty = true;
		}
	}
}
/*
SequenceEditor.prototype.keyDown = function(e) {
	switch (e.keyCode) {
		case 40: // down
			e.preventDefault();
			break;
		case 38: // up
			e.preventDefault();
			break;
		case 37: // 
			e.preventDefault();
			break;
		case 39:
			e.preventDefault();
			break;
		
	}
}
*/