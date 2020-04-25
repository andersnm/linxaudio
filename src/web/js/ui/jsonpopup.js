function JsonPopup() {
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
	
	function loadClick() {
		self.emit("load", self.jsonInput.value);
	}
		
	function resourceInputChange() {
		self.updateJsonResource();
	}

	this.container = document.createElement("overlay");
	this.container.addEventListener("click", overlayClick);
	this.container.addEventListener("keydown", overlayKeydown);

	this.popup = document.createElement("popup");	
	this.popup.tabIndex = 0;
	
	this.resourceWrap = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.resourceWrap, "absolute", "0", "0", "30%", "24px");
	
	this.resourceInput = document.createElement("select");
	this.resourceInput.style.padding = "0";
	this.resourceInput.style.margin = "0";
	this.resourceInput.style.width = "100%";
	this.resourceInput.style.height = "100%";
	this.resourceInput.size = 2;
	this.resourceInput.addEventListener("change", resourceInputChange);

	this.resourceWrap.appendChild(this.resourceInput);

	this.jsonWrap = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.jsonWrap, "absolute", "30%", "0", "0", "24px");
	
	this.jsonInput = document.createElement("textarea");
	this.jsonInput.style.padding = "0";
	this.jsonInput.style.margin = "0";
	this.jsonInput.style.width = "100%";
	this.jsonInput.style.height = "100%";
	this.jsonInput.style.whiteSpace = "pre";
	this.jsonInput.style.overflow = "auto";
	this.jsonInput.value = "";//JSON.stringify(testsynth, null, "  ");
	this.jsonWrap.appendChild(this.jsonInput);

	this.popup.appendChild(this.resourceWrap);
	this.popup.appendChild(this.jsonWrap);
	
	this.loadButton = document.createElement("input");
	this.loadButton.type  = "button";
	this.loadButton.value = "Load";
	this.loadButton.style.position = "absolute";
	this.loadButton.style.bottom = "0";
	this.loadButton.style.height = "24px";

	this.loadButton.addEventListener("click", loadClick);
	this.popup.appendChild(this.loadButton);
	
	this.container.appendChild(this.popup);
	
	Events(this);
}

JsonPopup.prototype.show = function() {
	
	this.updateResourceList();

	this.container.style.display = "block";
	this.resourceInput.focus();
}

JsonPopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}

JsonPopup.prototype.updateResourceList = function() {
	var resources = [];
	this.emit("requestResourceList", resources);
	HTMLSelectHelper.bindNameValueList(this.resourceInput, resources);

	if (resources.length > 0) {
		this.resourceInput.value = resources[0].value;
		this.updateJsonResource();
	}
}

JsonPopup.prototype.updateJsonResource = function() {
	var self = this;

	function requestResourceCallback(objectProject) {
		//console.log("JsonPopup got resource JSON");
		//console.log(objectProject);
		self.jsonInput.value = JSON.stringify(objectProject, null, "  ");
	}

	this.jsonInput.value = "";
	this.emit("requestResource", this.resourceInput.value, requestResourceCallback);
}
