// Module utility:
// "Wrap project in stereocontainer"
// "Wrap project in polycontainer"
// Song utility:
// "Create stereo master"


function UtilityPopup() {
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
			if (self.list.value) {
				self.emit("newModule", self.list.value);
				self.hide();
			}
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

	this.popup.innerHTML = "<h1>Utility Functions</h1>" + 
		"<p><a href=javascript:app.makeMaster();void(0);>Insert master</a><br />" +
		"Creates and wires up a stereo master module representing the browsers audio output. Useful to get started with a sequencing project</p>" +
		"<p><a href=javascript:app.wrapStereo();void(0);>Wrap project in stereo container</a><br />" +
		"Useful if you made a mono plugin and want to make it stereo</p>" +
		//"<a href=javascript:app.wrapPoly();void(0);>Wrap project in polyphonic container</a><br />" +
		//"Useful if you made a monophonic plugin and want to make it polyphonic" + 
		"";

	this.container.appendChild(this.popup);
	
	Events(this);

}

UtilityPopup.prototype.show = function() {
	this.container.style.display = "block";
	this.popup.focus();
}

UtilityPopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}

