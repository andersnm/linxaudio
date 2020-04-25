function SavePopupField(description, fieldName) {
	this.description = description;
	this.fieldName = fieldName;
}

function SavePopup() {
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
	
	function saveClick() {
		self.emit("save", self.project);
	}

	function shareClick() {
		self.emit("share", self.project);
	}
	
/*	function nameInputChange() {
		self.project.name = this.value;
		self.emit("updateProject", self.project);
	}

	function authorInputChange() {
		self.project.author = this.value;
		self.emit("updateProject", self.project);
	}

	function uniqueIdInputChange() {
		self.project.uniqueId = this.value;
		self.emit("updateProject", self.project);
	}
	*/
	this.project = null;

	this.container = document.createElement("overlay");
	this.container.addEventListener("click", overlayClick);
	this.container.addEventListener("keydown", overlayKeydown);

	this.popup = document.createElement("popup");	
	this.popup.tabIndex = 0;

	this.formContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.formContainer, "absolute", "0", "0", "0", "24px");
	this.formContainer.style.overflow = "auto";
	this.formContainer.style.padding = "10px";
	
/*	this.nameContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(this.nameContainer, "relative", "0", "0", "100%", "48px");

	this.nameLabel = document.createElement("div");
	this.nameLabel.innerHTML = "Name (primary key in local IndexedDB database):";
	
	this.nameInput = document.createElement("input");
	this.nameInput.type = "text";
	this.nameInput.addEventListener("change", nameInputChange);

	this.nameContainer.appendChild(this.nameLabel);
	this.nameContainer.appendChild(this.nameInput);
	

	this.authorContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(this.authorContainer, "relative", "0", "0", "100%", "48px");

	this.authorLabel = document.createElement("div");
	this.authorLabel.innerHTML = "Author:";
	
	this.authorInput = document.createElement("input");
	this.authorInput.type = "text";
	this.authorInput.addEventListener("change", authorInputChange);
	
	this.authorContainer.appendChild(this.authorLabel);
	this.authorContainer.appendChild(this.authorInput);
	
	this.uniqueIdContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(this.uniqueIdContainer, "relative", "0", "0", "100%", "48px");

	this.uniqueIdLabel = document.createElement("div");
	this.uniqueIdLabel.innerHTML = "Unique ID: (embedded in VST synths for identification in some hosts)";
	
	this.uniqueIdInput = document.createElement("input");
	this.uniqueIdInput.type = "text";
	this.uniqueIdInput.addEventListener("change", uniqueIdInputChange);
	
	this.uniqueIdContainer.appendChild(this.uniqueIdLabel);
	this.uniqueIdContainer.appendChild(this.uniqueIdInput);		
	
	this.formContainer.appendChild(this.nameContainer);
	this.formContainer.appendChild(this.authorContainer);
	this.formContainer.appendChild(this.uniqueIdContainer);*/
	
	this.popup.appendChild(this.formContainer);

	this.buttonPanel = document.createElement("div");
	HTMLBoxHelper.setLeftBottom(this.buttonPanel, "absolute", "0", "0", "100%", "24px");
	
	this.saveButton = document.createElement("input");
	this.saveButton.type  = "button";
	this.saveButton.value = "Save";
	this.saveButton.addEventListener("click", saveClick);

	this.shareButton = document.createElement("input");
	this.shareButton.type  = "button";
	this.shareButton.value = "Share";
	this.shareButton.addEventListener("click", shareClick);

	this.publicUrl = document.createElement("input");
	this.publicUrl.type = "text";
	this.publicUrl.readOnly = true;
	this.publicUrl.value = "";
	
	this.publicUrlLabel = document.createElement("span");
	this.publicUrlLabel.innerHTML = " via myjson.com ";

	this.publicUrlRawLink = document.createElement("a");
	this.publicUrlRawLink.href = "about:blank";
	this.publicUrlRawLink.target = "_blank";
	this.publicUrlRawLink.innerHTML = "(raw)";

	this.buttonPanel.appendChild(this.saveButton);
	this.buttonPanel.appendChild(this.shareButton);	
	this.buttonPanel.appendChild(this.publicUrl);
	this.buttonPanel.appendChild(this.publicUrlLabel);
	this.buttonPanel.appendChild(this.publicUrlRawLink);

	this.popup.appendChild(this.buttonPanel);

	this.container.appendChild(this.popup);
	
	Events(this);
}

SavePopup.prototype.show = function() {
	this.container.style.display = "block";
	if (this.formInputs.length > 0) {
		var firstInput = this.formInputs[0];
		firstInput.focus();
	} else {
		this.popup.focus();
	}
}

SavePopup.prototype.hide = function() {
	this.emit("updateProject", this.project);

	this.container.style.display = null;
	this.emit("hide");
}

SavePopup.prototype.createField = function(field) {

	var self = this;

	function fieldInputChange() {
		self.project[field.fieldName] = this.value;
		self.emit("updateProject", self.project);
	}

	var fieldContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(fieldContainer, "relative", "0", "0", "100%", "48px");

	var fieldLabel = document.createElement("div");
	fieldLabel.innerHTML = field.description; //"Name (primary key in local IndexedDB database):";
	
	var fieldInput = document.createElement("input");
	fieldInput.type = "text";
	fieldInput.addEventListener("change", fieldInputChange);
	fieldInput.value = this.project[field.fieldName];

	fieldContainer.appendChild(fieldLabel);
	fieldContainer.appendChild(fieldInput);

	this.formInputs.push(fieldInput);
	
	return fieldContainer;
}

SavePopup.prototype.bindProject = function(project, fields) {
	// NOTE: only view with dependency on graph domain objects! because i want to update the pro
	// TODO: project = list of custom fields to edit in the popup
	
	this.project = project;
	
	this.formContainer.innerHTML = "";
	this.formInputs = [];

	for (var i = 0; i < fields.length; i++) {
		var fieldContainer = this.createField(fields[i]);
		this.formContainer.appendChild(fieldContainer);
	}

	/*this.nameInput.value = project.name;
	this.authorInput.value = project.author;
	this.uniqueIdInput.value = project.uniqueId;*/
	
	if (project.myjsonId) {
		this.publicUrl.value = "http://anders-e.com/weblinx/designer.html?myjson=" + project.myjsonId;
		this.publicUrlRawLink.href = "http://myjson.com/" + project.myjsonId;
		this.publicUrlRawLink.style.display = "inline";
	} else {
		this.publicUrl.value = "";
		this.publicUrlRawLink.href = "about:blank";
		this.publicUrlRawLink.style.display = "none";
	}
}
