function PropagatePopup() {

	var self = this;

	function overlayClick(e) {
		if (e.target == self.container) {
			self.emit("propagate", self.getPropagations());
			self.hide();
		}
	}

	function overlayKeydown(e) {
		if (e.keyCode == 27) {
			self.hide();
			e.stopPropagation();
		}
	}
	
	function addClick() {
		var propagations = self.getPropagations();

		var moduleList = [];
		self.emit("requestModuleList", moduleList);
		
		if (moduleList.length == 0) {
			return ;
		}

		var pinList = [];
		self.emit("requestPinList", moduleList[0].value, pinList);

		var pinref = new LinxPinRef();
		pinref.vertex = moduleList[0].value;
		pinref.pin = pinList[0];

		propagations.push(pinref);
		
		self.bindPropagations(propagations);
	}
	
	this.container = document.createElement("overlay");
	this.container.addEventListener("click", overlayClick);
	this.container.addEventListener("keydown", overlayKeydown);
	
	this.popup = document.createElement("popup");
	this.popup.tabIndex = 0;
	
	this.tableContainer = document.createElement("div");
	this.tableContainer.style.position = "absolute";
	this.tableContainer.style.left = "0";
	this.tableContainer.style.top = "0";
	this.tableContainer.style.width = "100%";
	this.tableContainer.style.bottom = "24px";
	this.tableContainer.style.overflow = "auto";

	this.propagationTable = document.createElement("table");
	this.propagationTable.style.width ="100%";
	
	this.propagationTableBody = document.createElement("tbody");
	
	this.propagationTable.appendChild(this.propagationTableBody);
	this.tableContainer.appendChild(this.propagationTable);
	this.popup.appendChild(this.tableContainer);
	
	this.buttonPanel = document.createElement("div");
	this.buttonPanel.style.position = "absolute";
	this.buttonPanel.style.height = "24px";
	this.buttonPanel.style.left = "0";
	this.buttonPanel.style.bottom = "0";
	
	this.addButton = document.createElement("input");
	this.addButton.type = "button";
	this.addButton.value = "Add"
	this.addButton.addEventListener("click", addClick);
	
	this.buttonPanel.appendChild(this.addButton);
	this.popup.appendChild(this.buttonPanel);
	
	this.container.appendChild(this.popup);

	Events(this);
}

PropagatePopup.prototype.createRow = function(propagation, modules, pins) {

	var self = this;

	function moduleChange() {
		var pins = [];
		self.emit("requestPinList", moduleList.value, pins);
		HTMLSelectHelper.bindNameValueList(pinList, pins);
	}
	
	function deleteClick() {
		row.parentElement.removeChild(row);
	}

	var row = document.createElement("tr");
	var nameCell = document.createElement("td");
	var moduleCell = document.createElement("td");
	var pinCell = document.createElement("td");
	var opCell = document.createElement("td");
	
	var nameInput = document.createElement("input");
	nameInput.type = "text";
	nameInput.value = propagation.name;
	nameInput.style.width = "100%";
	nameCell.appendChild(nameInput);

	var moduleList = document.createElement("select");
	HTMLSelectHelper.bindNameValueList(moduleList, modules);
	moduleList.value = propagation.vertex;
	moduleList.style.width = "100%";
	moduleList.addEventListener("change", moduleChange);

	moduleCell.appendChild(moduleList);

	var pinList = document.createElement("select");
	HTMLSelectHelper.bindNameValueList(pinList, pins);
	pinList.value = propagation.pin;
	pinList.style.width = "100%";
	pinCell.appendChild(pinList);

	var deleteButton = document.createElement("input");
	deleteButton.type = "button";
	deleteButton.value = "Delete";
	deleteButton.addEventListener("click", deleteClick);
	opCell.appendChild(deleteButton);
	
	row.appendChild(nameCell);
	row.appendChild(moduleCell);
	row.appendChild(pinCell);
	row.appendChild(opCell);
	return row;
}

PropagatePopup.prototype.bindPropagations = function(propagations) {	
	this.propagationTableBody.innerHTML = "";
	
	var moduleList = [];
	this.emit("requestModuleList", moduleList);
	
	var self = this;
	propagations.forEach(function(propagation) {
		
		var pinList = [];
		self.emit("requestPinList", propagation.vertex, pinList);
		
		var row = self.createRow(propagation, moduleList, pinList);
		self.propagationTableBody.appendChild(row);
	});
}

PropagatePopup.prototype.getPropagations = function() {	
	var result = [];

	for (var i = 0; i < this.propagationTableBody.childNodes.length; i++) {
		var row = this.propagationTableBody.childNodes[i];
		var nameInput = row.childNodes[0].firstChild;
		var moduleList = row.childNodes[1].firstChild;
		var pinList = row.childNodes[2].firstChild;
		
		var pinref = new LinxPinRef();
		pinref.name = nameInput.value;
		pinref.vertex = moduleList.value;
		pinref.pin = pinList.value;
		result.push(pinref);
	}
	return result;
}

PropagatePopup.prototype.show = function() {
	this.container.style.display = "block";
	this.popup.focus();
}

PropagatePopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}

