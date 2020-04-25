function AboutDesignerPopup() {
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
		"<h1>About Designer</h1>" +
		"<p>Online audio synth/effect DSP graph editor and tracker/sequencer. The audio engine and DSP modules are compiled from C to asm.js for maximum performance and portability. The projects can be compiled to VST DLLs for desktop DAWs using desktop tools.</p>" +
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
		"<li>Ins/Del inserts or deletes a row in the current column</li>" +
		"<li>Ctrl+Enter to create a new pattern for the selected plugin</li>" +
		"<li>Num+/ and Num+* changes note input octave</li>" +
		"<li>Esc to go back to sequence editor</li>" +
		"</ul>" +
		"<h2>Graph Editor</h2>" + 
		"<ul>" +
		"<li>Start typing to open the \"Module Popup\" and create a new module. The module will be positioned at the last mouse click position</li>" +
		"<li>Shift+Drag between modules to open the \"Connection Popup\" and edit connected pins</li>" +
		"<li>Click a connection triangle to open the \"Connection Popup\" and edit connected pins</li>" +
		"<li>Shift+Drag the background to select multiple modules</li>" +
		"<li>Double click a module to open the \"Parameter Popup\" to rename and set parameters</li>" +
		"<li>Double click the special \"Propagate\" box to open the \"Propagate Popup\" to select pins to expose to the host or parent graph.</li>" +
		"<li>Press Enter to edit the selected module's subgraph, press Esc to leave</li>" +
		"<li>Ctrl+C/Ctrl+X/Ctrl+V to Copy/Cut/Paste selected modules</li>" +
		"</ul>" +
		"<h2>Sequence Editor</h2>" + 
		"<ul>" +
		"<li>0..9 to insert respective pattern index</li>" +
		"<li>Ins/Del inserts or deletes a row in the current column</li>" +
		"<li>Enter to go to the pattern under the cursor</li>" +
		"</ul>" +
		"<h2>Loading and saving</h2>" + 
		"<p>Projects are saved locally in the browser using IndexedDB. The \"Share\" feature saves the project behind the scenes to myjson.com and provides a redistributable link to import the project in another browser.</p>" +
		"<h2>Pin propagation</h2>" + 
		"<p>Any vertex pins in a graph can be propagated. Pins propagated from a subgraph will appear as regular pins on the containing vertex in the parent graph. Whereas pins propagated from the root graph will appear in the host depending on the pin type and direction. </p>" +
		"<h2>Audio engine</h2>" + 
		"<p>\"linxaudio\" consists of a DSP graph engine, a standard module library and a cross platform command line tool \"linxgen.exe\". The tool generates VST and Buzz audio plugins from DSP graph descriptions in JSON format and precompiled modules without a compiler or full development environment.</p>" +
		"<p>\"linx.js\" is the DSP graph engine, standard modules and designer extensions compiled to Javascript/asm.js with Emscripten. Plus custom Javascript code to glue everything together.</p>" +
		"<h2>Advanced/Debug</h2>" + 
		"<a href=javascript:app.deleteDatabase();void(0);>Delete database</a> " +
		"<a href=javascript:app.importDefaultProjects();void(0);>Reimport default plugins</a>" +
		"";
	
	this.container.appendChild(this.popup);
	
	Events(this);
}

AboutDesignerPopup.prototype.show = function() {
	this.container.style.display = "block";
	this.popup.focus();
}

AboutDesignerPopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}
