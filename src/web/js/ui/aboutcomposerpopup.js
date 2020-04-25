function AboutComposerPopup() {
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
		}
	}
	
	this.container = document.createElement("overlay");
	this.container.addEventListener("click", overlayClick);
	this.container.addEventListener("keydown", overlayKeydown);

	this.popup = document.createElement("popup");	
	this.popup.tabIndex = 0;
	this.popup.style.overflow = "auto";
	this.popup.style.padding = "10px";
	this.popup.innerHTML = 
		"<h1>About Composer</h1>" +
		"<p>Modular tracker. Supports synths and effects made with Designer, and some built-in Web Audio-based plugins.</p>" +
		"<p style=color:red>This is BETA software. Everything is under massive development and will change without notification.</p>" +
		"<h2>Usage</h2>" + 
		"<ul>" +
		"<li>F1 to show this \"About Popup\" for basic help</li>" +
		"<li>F2 to show the pattern editor</li>" +
		"<li>F3 to show the graph editor</li>" +
		"<li>F4 to show the sequence editor</li>" +
		"<li>F5 to play/resume</li>" +
		"<li>F6 to play from cursor in sequence editor</li>" +
		"<li>F8 to stop</li>" +
		"<li>Ctrl+L to show the \"Load Popup\" to import a project</li>" +
		"<li>Ctrl+S to show the \"Save Popup\" to save and share</li>" +
		"</ul>" +
		"<p>Limitations: Pattern and sequence editors work, but the working area cannot exceed the maximum area supported by the canvas element. Typically around ~2000 rows at 16px font height. </p>" +
		"<h2>Pattern Editor</h2>" + 
		"<ul>" +
		"<li>Pattern values can be notes, integers or floating point numbers. All numbers are shown in 10-base</li>" +
		"<li>Ctrl+Enter to create a new pattern for the selected plugin</li>" +
		"<li>Num+/ and Num+* changes note input octave</li>" +
		"<li>Esc to go back to sequence editor</li>" +
		"</ul>" +
		"<h2>Graph Editor</h2>" + 
		"<ul>" +
		"<li>Shift+Drag between plugins to open the \"Connection Popup\" and edit connection</li>" +
		"<li>Double click a plugin to open the \"Parameter Popup\" to rename and set parameters</li>" +
		"<li>Start typing to open the \"Plugin Popup\" and create a new plugin instance. The plugin will be positioned at the last mouse click position</li>" +
		"</ul>" +
		"<h2>Sequence Editor</h2>" + 
		"<ul>" +
		"<li>0..9 to insert respective pattern index</li>" +
		"<li>Enter to go to the pattern under the cursor</li>" +
		"</ul>" +
		"<h2>Loading and saving</h2>" + 
		"<p>Projects are saved locally in the browser using IndexedDB. The \"Share\" feature saves the project behind the scenes to myjson.com and provides a redistributable link to import the project in another browser.</p>" +
		"";
	
	this.container.appendChild(this.popup);
	
	Events(this);
}

AboutComposerPopup.prototype.show = function() {
	this.container.style.display = "block";
	this.popup.focus();
}

AboutComposerPopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}
