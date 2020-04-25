/*
PE BG			DAD6C9
PE BG Dark		BDB59F
PE BG Very Dark		9F9373
PE Sel BG		F7F7F4
PE Text			303021
PE Text Shade	686868
PE Text Note	202080

*/
var TOOLBAR_HEIGHT = 24;


function PatternEditorCursorColumn(viewColumnIndex, x, valueIndex) {
	this.x = x;
	this.viewColumnIndex = viewColumnIndex;
	this.valueIndex = valueIndex;
}

function PatternEditorBaseColumn(vertex, name, pinType, type, minValue, maxValue) {
	this.vertex = vertex;
	this.name = name;
	this.pinType = pinType; // 4 for midi, 0 for float, 1 for integer
	this.type = type; // type: one of "midi", "byte", "word", "float"
	this.minValue = minValue;
	this.maxValue = maxValue;
}

function PatternEditorViewColumn(baseColumnIndex, compoundIndex, x, label, type, width, minValue, maxValue) {
	this.baseColumnIndex = baseColumnIndex;
	this.cursorColumnIndex = 0; // updated later
	this.compoundIndex = compoundIndex; // used with notes; note = 0, octave = 1
	this.label = label;
	this.type = type;	// one of "byte", "word", "float",  (not midi, its compound of several bytes)
	this.minValue = minValue;
	this.maxValue = maxValue;
	this.width = width;
	this.x = x; // updated later
}

function PatternEditor(mainContainer) {

	this.skipRows = 1;
	this.stepSize = 1;
	this.inputNoteOctave = 4;
	this.cursorColumnIndex = 0;
	this.cursorScreenRow = 0;	
	this.fontWidth = 9;
	this.fontHeight = 16;
	this.dirty = true;

	this.containerIndex = -1;
	this.patternIndex = -1;

	this.baseColumns = [];
	this.viewColumns = [];
	this.cursorColumns = [];

	this.container = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.container, "absolute", "0", "0", "0", "0");

	this.initToolbar();
	this.initEditor();

	mainContainer.appendChild(this.container);
	
	Events(this);
}

PatternEditor.prototype.initToolbar = function() {

	var self = this;

	function skipRowsChange() {
		self.skipRows = parseInt(self.skipRowsList.value);
		self.updateCanvas();
	}

	function stepSizeChange() {
		self.stepSize = parseInt(self.stepSizeList.value);
	}

	function pluginChange(e) {
		self.updatePatternList();
		self.emit("selectPattern", self.pluginList.value, self.patternList.value);
	}
	
	function patternChange() {
		self.emit("selectPattern", self.pluginList.value, self.patternList.value);
	}

	this.toolbarContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(this.toolbarContainer, "absolute", "0", "0", "100%", TOOLBAR_HEIGHT + "px");
	this.toolbarContainer.style.backgroundColor = "#dddddd";

	this.pluginList = document.createElement("select");
	this.pluginList.addEventListener("change", pluginChange);
	
	this.patternList = document.createElement("select");
	this.patternList.addEventListener("change", patternChange);

	this.toolbarContainer.appendChild(document.createTextNode("Plugin: "));
	this.toolbarContainer.appendChild(this.pluginList);

	this.toolbarContainer.appendChild(document.createTextNode(" Pattern: "));
	this.toolbarContainer.appendChild(this.patternList);

	this.toolbarContainer.appendChild(document.createTextNode(" Skip Rows: "));
	
	this.skipRowsList = document.createElement("select");
	for (var i = 0; i < 64; i++) {
		var opt = document.createElement("option");
		opt.text = i + 1;
		opt.value = i + 1;
		this.skipRowsList.appendChild(opt);
	}
	this.skipRowsList.value = this.skipRows;
	this.skipRowsList.addEventListener("change", skipRowsChange);
	this.toolbarContainer.appendChild(this.skipRowsList);

	
	this.toolbarContainer.appendChild(document.createTextNode(" Step Size: "));	
	this.stepSizeList = document.createElement("select");
	for (var i = 0; i < 64; i++) {
		var opt = document.createElement("option");
		opt.text = i + 1;
		opt.value = i + 1;
		this.stepSizeList.appendChild(opt);
	}
	this.stepSizeList.value = this.stepSize;
	this.stepSizeList.addEventListener("change", stepSizeChange);
	this.toolbarContainer.appendChild(this.stepSizeList);


	this.container.appendChild(this.toolbarContainer);
}

PatternEditor.prototype.moveCursor = function(x, y) {
	this.renderCursor();
	this.cursorColumnIndex += x;
	if (this.cursorColumnIndex < 0) {
		this.cursorColumnIndex = 0;
	}
	if (this.cursorColumnIndex >= this.cursorColumns.length) {
		this.cursorColumnIndex = this.cursorColumns.length - 1;
	}
	
	this.cursorScreenRow += y;
	if (this.cursorScreenRow < 0) {
		this.cursorScreenRow = 0;
	}
	
	var totalScreenRows = Math.floor(this.rows / this.skipRows);
	if (this.cursorScreenRow >= totalScreenRows) {
		this.cursorScreenRow = totalScreenRows - 1;
	}

	this.renderCursor();
	this.scrollToView();
}

PatternEditor.prototype.setCursorDecimal = function() {
	var cursorColumn = this.cursorColumns[this.cursorColumnIndex];
	var viewColumn = this.viewColumns[cursorColumn.viewColumnIndex];
	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];
	
	var baseValue = this.getCurrentPatternValue(baseColumn);
	var viewValue = this.getCompoundValue(viewColumn, baseValue);
	
	console.log("setCrsordecimal", baseValue, viewValue);

	if (viewValue != null && cursorColumn.valueIndex < viewColumn.width - 1) {
		var viewValueString = formatValue(viewColumn, viewValue, "0");
		var cp = viewValueString.indexOf(".");
		if (cp != -1) {
			// move decimal
			viewValueString = viewValueString.replace(".", "");
			viewValueString = insertAt(viewValueString, cursorColumn.valueIndex, ".");
		} else {
			// insert decimal
			viewValueString = setCharAt(viewValueString, cursorColumn.valueIndex, ".");
		}	

		viewValue = parseFloat(viewValueString);
		baseValue = this.setCompoundValue(viewColumn, baseValue, viewValue);

		this.emit("setPatternValue", this.containerIndex, this.patternIndex, baseColumn, this.cursorScreenRow * this.skipRows, baseValue);
		this.dirty = true;
	}
}

PatternEditor.prototype.negateCursorValue = function(value) {
	// TODO: minmax

	var cursorColumn = this.cursorColumns[this.cursorColumnIndex];
	var viewColumn = this.viewColumns[cursorColumn.viewColumnIndex];
	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];

	var baseValue = this.getCurrentPatternValue(baseColumn);
	var viewValue = this.getCompoundValue(viewColumn, baseValue);
	if (viewValue) {
		viewValue = -viewValue;
		
		baseValue = this.setCompoundValue(viewColumn, baseValue, viewValue);
		console.log("new value: " + baseValue);
		
		this.emit("setPatternValue", this.containerIndex, this.patternIndex, baseColumn, this.cursorScreenRow * this.skipRows, baseValue);
		this.dirty = true;
	}
}

PatternEditor.prototype.setCurrentPatternValue = function(baseColumn, value) {
	this.emit("setPatternValue", this.containerIndex, this.patternIndex, baseColumn, this.cursorScreenRow * this.skipRows, value);
}

PatternEditor.prototype.setCompoundValue = function(viewColumn, baseValue, value) {
	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];
	if (baseColumn.type == "midi") {
		if (viewColumn.compoundIndex == 0) {
			baseValue.degree = value;
		} else if (viewColumn.compoundIndex == 1) {
			baseValue.type = value;
		} else if (viewColumn.compoundIndex == 2) {
			baseValue.velocity = value;
		}
		if (baseValue.degree == null && baseValue.type == null && baseValue.velocity == null) {
			return null;
		} else {
			return baseValue;
		}
	} else {
		return value;
	}
}

PatternEditor.prototype.getCompoundValue = function(viewColumn, baseValue) {
	if (baseValue == null) {
		return null;
	}

	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];
	if (baseColumn.type == "midi") {
		var note = baseValue;
		if (viewColumn.compoundIndex == 0) {
			return note.degree;
		} else if (viewColumn.compoundIndex == 1) {
			return note.type;
		} else if (viewColumn.compoundIndex == 2) {
			return note.velocity;
		}
	} else {
		return baseValue;
	}
}

PatternEditor.prototype.getEmptyPatternValue = function(baseColumn) {
	if (baseColumn.type == "midi") {
		return { degree: null, type : null, velocity : null };
	} else {
		return null;
	}
}

PatternEditor.prototype.getCurrentPatternValue = function(baseColumn) {
	var row = this.cursorScreenRow * this.skipRows;
	var values = [];
	this.emit("requestPatternValues", this.containerIndex, this.patternIndex, baseColumn, row, row + 1, values);

	if (values.length == 0) {
		return this.getEmptyPatternValue(baseColumn);
	} else {
		return values[0].value;
	}
}

PatternEditor.prototype.editCursorNibble = function(value) {
	// value must be a number between 0..9

	// TODO: handle negative numbers
	// TODO: minmax

	var cursorColumn = this.cursorColumns[this.cursorColumnIndex];
	var viewColumn = this.viewColumns[cursorColumn.viewColumnIndex];
	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];

	var baseValue = this.getCurrentPatternValue(baseColumn);
	var viewValue = this.getCompoundValue(viewColumn, baseValue);

	var sourceValueString;
	if (viewValue) {
		viewValueString = formatValue(viewColumn, viewValue, "0");
	} else {
		viewValueString = new Array(viewColumn.width + 1).join("0");/*"0";
		while (sourceValueString.length < viewColumn.width)
			sourceValueString = "0" + sourceValueString;*/
	}

	viewValueString = setCharAt(viewValueString, cursorColumn.valueIndex, value.toString());
	viewValue = parseFloat(viewValueString);
	
	baseValue = this.setCompoundValue(viewColumn, baseValue, viewValue);
	
	this.setCurrentPatternValue(baseColumn, baseValue);
	//this.emit("setPatternValue", this.pattern, baseColumn, this.cursorScreenRow * this.skipRows, baseValue);

	this.dirty = true;
}

PatternEditor.prototype.deleteAtCursor = function() {
	var cursorColumn = this.cursorColumns[this.cursorColumnIndex];
	var viewColumn = this.viewColumns[cursorColumn.viewColumnIndex];
	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];
	
	this.emit("setPatternValue", this.containerIndex, this.patternIndex, baseColumn, this.cursorScreenRow * this.skipRows, null);
	this.dirty = true;
}

PatternEditor.prototype.show = function() {
	this.container.style.display = "block";
	//this.updateCanvas();
	this.editorContainer.focus();
}

PatternEditor.prototype.hide = function() {
	this.container.style.display = "none";
}

PatternEditor.prototype.initEditor = function() {
	var self = this;

	function containerScroll(e) {
		e.preventDefault();
		self.rowNumberContainer.scrollTop = self.canvasContainer.scrollTop;
		self.headerContainer.scrollLeft = self.canvasContainer.scrollLeft;
	}
	
	function editorKeyDown(e) {
		self.editorKeyDown(e);
	}

	function editorKeyPress(e) {
		self.editorKeyPress(e);
	}
	
	function editorMouseDown(e) {
		self.editorMouseDown(e);
	}

	this.editorContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.editorContainer, "absolute", "0px", TOOLBAR_HEIGHT + "px", "0", "0");
	this.editorContainer.style.backgroundColor = "#DAD6C9";
	this.editorContainer.tabIndex = "0";
	this.editorContainer.addEventListener("keydown", editorKeyDown);
	this.editorContainer.addEventListener("keypress", editorKeyPress);
	this.editorContainer.addEventListener("mousedown", editorMouseDown);

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
	this.context = this.canvas.getContext("2d");
	this.canvasContainer.appendChild(this.canvas);

	this.editorContainer.appendChild(this.canvasContainer);
	this.container.appendChild(this.editorContainer);
}

PatternEditor.prototype.updateCanvas = function() {
	if (this.patternIndex == -1) {
		return ;
	}

	this.context.font = "12px monospace";
	var size = this.context.measureText("W");
	this.fontWidth = size.width;
	
	var lastColumn = this.viewColumns[this.viewColumns.length - 1];
	var totalColumnWidth = lastColumn != null ? (lastColumn.x + lastColumn.width) : 0;
	
	 // if the canvas is less than some width, the cursor fillRect w/difference becomes all black for some reason! (on chrome)
	var minCanvasWidth = 96;
	this.canvas.width = Math.max(totalColumnWidth * this.fontWidth, minCanvasWidth);
	this.canvas.height = this.rows / this.skipRows * this.fontHeight;
	
	this.rowNumberContainer.innerHTML = "";
	for (var i = 0 ; i < this.rows / this.skipRows; i++) {
		var rowNumber = document.createElement("div");
		rowNumber.style.height = "16px";
		rowNumber.style.paddingRight = "2px";
		rowNumber.innerHTML = (i * this.skipRows).toString();
		this.rowNumberContainer.appendChild(rowNumber);
	}
	
	this.headerContainer.style.width = "100%";
	this.headerContainer.innerHTML = "";
	
	var self = this;
	this.viewColumns.forEach(function(viewColumn, index) {
		var x = 64 + viewColumn.x * self.fontWidth; //(index + 1) * 64;
		var width = viewColumn.width * self.fontWidth;
		var trackHeader = document.createElement("div");
		HTMLBoxHelper.setLeftTop(trackHeader, "absolute", x + "px", "0px", width + "px", "24px");
		trackHeader.style.overflow = "hidden";
		trackHeader.style.borderRight = "1px solid black";
		trackHeader.innerHTML = viewColumn.label;
		self.headerContainer.appendChild(trackHeader);
		
	});
	
	this.dirty = true;
}

PatternEditor.prototype.scrollToView = function() {
	var cursorRow = this.cursorScreenRow;
	var scrollRow = Math.floor(this.canvasContainer.scrollTop / this.fontHeight);
	var screenRows = Math.floor(this.canvasContainer.offsetHeight / this.fontHeight) - 1;
	if (cursorRow > screenRows + scrollRow) {
		this.canvasContainer.scrollTop = (cursorRow - screenRows) * this.fontHeight;
	} else if (cursorRow < scrollRow) {
		this.canvasContainer.scrollTop = cursorRow * this.fontHeight;
	}
}

// http://stackoverflow.com/questions/1431094/how-do-i-replace-a-character-at-a-particular-index-in-javascript
function setCharAt(str, index, chr) {
	if (index > str.length - 1) return str;
	return str.substr(0, index) + chr + str.substr(index + 1);
}

// http://stackoverflow.com/questions/4364881/inserting-string-at-position-x-of-another-string
function insertAt(str, index, instr) {
	return str.substr(0, index) + instr + str.substr(index);
}

function parseMidi(message) {
	return {
		status : message & 0xff,
		channel : message & 0x0f,
		command : (message & 0xf0) >> 4,
		data1 : (message >> 8) & 0xff,
		data2 : (message >> 16) & 0xff,
	};
}

function makeMidi(midiEvent) {
	return ((midiEvent.data2 & 0xff) << 16) | ((midiEvent.data1 & 0xff) << 8) | ((midiEvent.command & 0x0f) << 4) | (midiEvent.channel & 0x0f);	
}

function formatValue(viewColumn, value, fillChar) {
	var noteKeyNames = [ "C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"];

	if (viewColumn.type == "note") {
		if (value == null) {
			return "---";
		} else {
			if (value == 255) {
				return "off";
			} else {
				var midinote = value;
				var note = midinote % 12;
				var octave = Math.floor(midinote / 12);
				//console.log(note + "; " + octave);
				return noteKeyNames[note] + octave.toString();

			}
		}
	} else {
		if (value == null) {
			return new Array(viewColumn.width + 1).join(".");
		} else {
			var length = viewColumn.width;
			if (value < 0) {
				length -= 1;
			}
			var result = Math.abs(value).toString();
			while (result.length < length) {
				result = fillChar + result;
			}
			if (value < 0) {
				result = "-" + result;
			}
			//console.log("the resut: ", 
			return result;
		}
	}
}

PatternEditor.prototype.renderTrack = function(viewColumn) {
	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];

	var x = viewColumn.x * this.fontWidth;
	var width = viewColumn.width * this.fontWidth;
	
	this.context.fillStyle = "#303021";
	this.context.font = "12px monospace";
	
	var patternEvents = []; // timestamp, value (value is baseColumn-based and abstracted through the event)
	this.emit("requestPatternValues", this.containerIndex, this.patternIndex, baseColumn, 0, this.rows, patternEvents);

	for (var i = 0; i < this.rows; i++) {
	
		var y = i * this.fontHeight;
		var rowIndex = i * this.skipRows;
		
		if ((rowIndex % 16) == 0) {
			this.context.fillStyle = "#9F9373";
		} else if ((rowIndex % 4) == 0) {
			this.context.fillStyle = "#BDB59F";
		} else {
			this.context.fillStyle = "#DAD6C9";
		}
		this.context.fillRect(x, y, width, this.fontHeight);
		
		this.context.fillStyle = "#303021";
		
		var patternEvent = patternEvents.find(function(patternEvent) {
			return patternEvent.timestamp == rowIndex;
		});
		
		var baseValue = patternEvent ? patternEvent.value : this.getEmptyPatternValue(baseColumn);
		var viewValue = this.getCompoundValue(viewColumn, baseValue);

		var formattedValue = formatValue(viewColumn, viewValue, " ");
		
		this.context.fillText(formattedValue, x, 11 + y);
	}
}

PatternEditor.prototype.render = function() {
	var self = this;
	
	if (!this.dirty || this.patternIndex == -1) {
		return ;
	}

	this.context.fillStyle = "#DAD6C9";
	this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);

	this.viewColumns.forEach(function(viewColumn) {
		self.renderTrack(viewColumn);
	});

	this.renderCursor();
	this.dirty = false;
}

PatternEditor.prototype.renderCursor = function() {
	// NOTE: render cursor once and it appears, render cursor twice and its gone again
	// NOTE: rounding cursor pos/sise to avoid artifacts with antialiasing and difference mode on chrome
	// (this.fontWidth is float result from measureText)
	//console.log("cursor at", this.cursorColumnIndex);
	var cursorColumn = this.cursorColumns[this.cursorColumnIndex];
	var width = Math.ceil(this.fontWidth);
	var x = Math.round(cursorColumn.x * this.fontWidth);

	this.context.globalCompositeOperation = "difference";
	this.context.fillStyle = "#ffffff";		
	this.context.fillRect(x, this.cursorScreenRow * this.fontHeight, width, this.fontHeight);
	this.context.globalCompositeOperation = "source-over";
	//console.log(x, this.cursorScreenRow * this.fontHeight, width, this.fontHeight);
}

PatternEditor.prototype.updateContainerList = function() {
	var pluginItems = [];
	var value = this.pluginList.value;
	this.emit("requestContainerList", pluginItems);
	HTMLSelectHelper.bindNameValueList(this.pluginList, pluginItems);
	this.pluginList.value = value;
}

PatternEditor.prototype.updatePatternList = function() {
	var patternItems = [];
	var value = this.patternList.value;
	this.emit("requestPatternList", this.pluginList.value, patternItems);
	HTMLSelectHelper.bindNameValueList(this.patternList, patternItems);
	this.patternList.value = value;
}

PatternEditor.prototype.addNoteColumn = function(compoundIndex, label) {
	var x = this.nextCursorPositionX;	
	var baseColumnIndex = this.baseColumns.length;
	var viewColumnIndex = this.viewColumns.length;
	
	this.viewColumns.push(new PatternEditorViewColumn(baseColumnIndex, compoundIndex, x, label, "note", 3, 0, 127));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x, 0));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 2, 1));

	this.nextCursorPositionX += 3;
}

PatternEditor.prototype.addByteColumn = function(compoundIndex, label, minValue, maxValue) {
	var x = this.nextCursorPositionX;
	var baseColumnIndex = this.baseColumns.length;
	var viewColumnIndex = this.viewColumns.length;
	var cursorColumnIndex = this.cursorColumns.length;

	this.viewColumns.push(new PatternEditorViewColumn(baseColumnIndex, compoundIndex, x, label, "byte", 3, minValue, maxValue));

	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 0, 0));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 1, 1));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 2, 2));

	this.nextCursorPositionX += 3;
}

PatternEditor.prototype.addWordColumn = function(compoundIndex, label, minValue, maxValue) {
	var x = this.nextCursorPositionX;
	var baseColumnIndex = this.baseColumns.length;
	var viewColumnIndex= this.viewColumns.length;

	this.viewColumns.push(new PatternEditorViewColumn(baseColumnIndex, compoundIndex, x, label, "word", 5, minValue, maxValue));

	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 0, 0));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 1, 1));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 2, 2));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 3, 3));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 4, 4));

	this.nextCursorPositionX += 5;
}

PatternEditor.prototype.addFloatColumn = function(compoundIndex, label, minValue, maxValue) {
	// 6 digits including decimal separator and sign "-12.45"
	var x = this.nextCursorPositionX;
	var baseColumnIndex = this.baseColumns.length;
	var viewColumnIndex= this.viewColumns.length;

	this.viewColumns.push(new PatternEditorViewColumn(baseColumnIndex, compoundIndex, x, label, "float", 6, minValue, maxValue));

	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 0, 0));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 1, 1));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 2, 2));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 3, 3));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 4, 4));
	this.cursorColumns.push(new PatternEditorCursorColumn(viewColumnIndex, x + 5, 5));

	this.nextCursorPositionX += 6;
}

PatternEditor.prototype.createPatternEditor = function(containerIndex, patternIndex, rows) {
	var self = this;
	console.log("createPatternEditor", containerIndex, patternIndex, rows);

	this.containerIndex = containerIndex;
	this.patternIndex = patternIndex;
	this.rows = rows;
	this.baseColumns = [];
	this.viewColumns = [];
	this.cursorColumns = [];
	this.cursorColumnIndex = 0;
	this.cursorScreenRow = 0;
	
	if (this.patternIndex == -1) {
		return ;
	}
	
	var baseColumns = [];
	this.emit("requestBaseColumns", containerIndex, patternIndex, baseColumns);

	this.nextCursorPositionX = 0;

	// TODO: addNoteColumn skal også adde viewColumn, dvs, addMidiColumn skal adde compound
	
	baseColumns.forEach(function(baseColumn) {
		// add them cursor columns
		// console.log("ADDING BASE!", baseColumn);
		//baseColumn.x = self.nextCursorPositionX;
		//baseColumn.cursorColumnIndex = self.cursorColumns.length;

		if (baseColumn.type == "midi") {
			self.addByteColumn(0, baseColumn.name + " Degree", -14, 14);
			self.addByteColumn(1, baseColumn.name + " Type", 0, 4);
			self.addByteColumn(2, baseColumn.name + " Velocity", 0, 127);
		} else if (baseColumn.type == "byte") {
			self.addByteColumn(0, baseColumn.name, baseColumn.minValue, baseColumn.maxValue);
		} else if (baseColumn.type == "word") {
			self.addWordColumn(0, baseColumn.name, baseColumn.minValue, baseColumn.maxValue);
		} else if (baseColumn.type == "float") {
			self.addFloatColumn(0, baseColumn.name, baseColumn.minValue, baseColumn.maxValue);
		} else {
			console.log("NO FLS " + baseColumn.type);
		}
		self.nextCursorPositionX += 1;
		self.baseColumns.push(baseColumn);
	});

	this.updateCanvas();
	this.dirty = true;
}

PatternEditor.prototype.editorKeyDown = function(e) {

	if (!this.cursorColumns.length) {
		return;
	}

	var cursorColumn = this.cursorColumns[this.cursorColumnIndex];
	var viewColumn = this.viewColumns[cursorColumn.viewColumnIndex];
	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];

	// note keycodes, arrows
	switch (e.keyCode) {
		case 9:
			console.log("TABIN!");
			if (e.shiftKey) {
				// prev col
				if (cursorColumn.viewColumnIndex > 0) {
					var nextCol = this.viewColumns[cursorColumn.viewColumnIndex - 1];
					this.moveCursor(nextBaseCol.cursorColumnIndex - this.cursorColumnIndex, 0);
				}
			} else {
				// next col
				if (cursorColumn.viewColumnIndex < this.viewColumns.length - 1) {
					var nextCol = this.viewColumns[cursorColumn.viewColumnIndex + 1];
					this.moveCursor(nextCol.cursorColumnIndex - this.cursorColumnIndex, 0);
				}
			}
			e.preventDefault();
			break;
		case 40: this.moveCursor(0, 1); return ;
		case 38: this.moveCursor(0, -1); return;
		case 37: this.moveCursor(-1, 0); return ;
		case 39: this.moveCursor(1, 0); return ;
		case 33: this.moveCursor(0, -16); return ;
		case 34: this.moveCursor(0, 16); return ;
		case 36:  // home
			if (e.ctrlKey) {
				this.moveCursor(0, -this.cursorScreenRow); 
			} else {
				this.moveCursor(-this.cursorColumnIndex, 0); 
			}
			return ;
		case 35:  // end
			if (e.ctrlKey) {
				// TODO: last row
			} else {
				this.moveCursor(this.cursorColumns.length - 1 - this.cursorColumnIndex, 0); 
			}
			return ;
		case 45: // insert
			this.emit("shiftPattern", this.containerIndex, this.patternIndex, baseColumn, this.cursorScreenRow * this.skipRows, this.skipRows);
			this.dirty = true;
			break;
		case 46: // delete
			this.deleteAtCursor();
			this.emit("shiftPattern", this.containerIndex, this.patternIndex, baseColumn, this.cursorScreenRow * this.skipRows, -this.skipRows);
			this.dirty = true;
			break;
		case 8: // backspace
			break;
		case 13:
			if (e.shiftKey) {
				this.emit("newCopyPattern", this.pluginList.value, this.patternList.value);
			} else if (e.ctrlKey) {
				this.emit("newPattern", this.pluginList.value);
			}
			break;
		case 111:
			if (this.inputNoteOctave > 0) {
				this.inputNoteOctave --;
			}
			this.emit("changeOctave");
			break;
		case 106:
			if (this.inputNoteOctave < 8) {
				this.inputNoteOctave ++;
			}
			this.emit("changeOctave");
			break;
		case 27:
			this.emit("leavePattern");
			break;
	}
	
	if (e.ctrlKey) {
		return ;
	}
	
	var noteKeyCodes = [
		90, 83, 88, 68, 67, 86, 71, 66, 72, 78, 74, 77,
		81, 50, 87, 51, 69, 82, 53, 84, 54, 89, 55, 85
	];
	
	if (baseColumn.type == "note") {
		var keyIndex = noteKeyCodes.indexOf(e.keyCode);
		if (keyIndex != -1) {

			var note = keyIndex % 12;
			var octave = this.inputNoteOctave + Math.floor(keyIndex / 12);
			var value;
			value = octave * 12 + note;

			this.emit("setPatternValue", this.containerIndex, this.patternIndex, baseColumn, this.cursorScreenRow * this.skipRows, value);
			
			this.moveCursor(0, this.stepSize);
			this.dirty = true;
			e.preventDefault();
			return ;
		}
	}
}

PatternEditor.prototype.editorKeyPress = function(e) {
	var cursorColumn = this.cursorColumns[this.cursorColumnIndex];
	var viewColumn = this.viewColumns[cursorColumn.viewColumnIndex];
	var baseColumn = this.baseColumns[viewColumn.baseColumnIndex];
	
	if (baseColumn.type == "note") {
		if (e.charCode == 49) { // 1
			this.emit("setPatternValue", this.containerIndex, this.patternIndex, baseColumn, this.cursorScreenRow * this.skipRows, 255);
			this.moveCursor(0, this.stepSize);
			this.dirty = true;
		}
	} else {
		if (e.charCode >= 48 && e.charCode <= 57) {
			this.editCursorNibble(e.charCode - 48);
			this.moveCursor(0, this.stepSize);
		} else if (e.charCode == 46) {
			this.deleteAtCursor();
		} else if (e.charCode == 45) {
			this.negateCursorValue();
		} else if (e.charCode == 44) {
			this.setCursorDecimal();
			//console.log("comma?")
		}
	}

}

PatternEditor.prototype.editorMouseDown = function(e) {
	// screen to cursor
	var offset = HTMLElementHelper.offset(this.canvas);
	var x = (e.pageX - offset.left);
	var y = (e.pageY - offset.top); 
	
	var screenColumn = Math.floor(x / this.fontWidth);
	var cursorColumnIndex = -1;
	// lookup x in this.cursorColumns
	for (var i = 0; i < this.cursorColumns.length; i++) {
		var cursorColumn = this.cursorColumns[i];
		if (cursorColumn.x >= screenColumn) {
			cursorColumnIndex = i;
			break;
		}
	}
	
	if (cursorColumnIndex != -1) {
		var row = Math.floor(y / this.fontHeight);
		row = Math.max(0, row);

		this.cursorScreenRow = row;
		this.cursorColumnIndex = cursorColumnIndex;
		this.dirty = true;
	}
}
