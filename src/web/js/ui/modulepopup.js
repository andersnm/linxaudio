function ModulePopup() {
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
	
	function filterTextboxKey(e) {
		if (e.keyCode == 40) {
			if (!self.list.value) {
				self.list.value = self.list.options[0].value;
			}
			self.list.focus();
		}
	}
	
	function filterTextboxChange(e) {
		self.refresh();
	}
	
	function categoryListChange(e) {
		self.refresh();
	}
	
	function listDoubleClick(e) {
		if (self.list.value) {
			self.emit("newModule", self.list.value);
			self.hide();
		}
	}
	
	function listChange(e) {
		self.updateModuleInfo(self.list.value);
	}

	this.container = document.createElement("overlay");
	this.container.addEventListener("click", overlayClick);
	this.container.addEventListener("keydown", overlayKeydown);
	
	this.popup = document.createElement("popup");
	this.popup.tabIndex = 0;
	
	this.filterTextbox = document.createElement("input");
	this.filterTextbox.type = "text";
	this.filterTextbox.style.position = "absolute";
	this.filterTextbox.style.width = "100%";
	this.filterTextbox.style.height = "24px";
	this.filterTextbox.addEventListener("keydown", filterTextboxKey);
	this.filterTextbox.addEventListener("keyup", filterTextboxChange);
	this.popup.appendChild(this.filterTextbox);

	this.categoryList = document.createElement("select");
	this.categoryList.size = 2;
	this.categoryList.style.position = "absolute";
	this.categoryList.style.left = "0";
	this.categoryList.style.top = "24px";
	this.categoryList.style.width = "33%";
	this.categoryList.style.bottom = "50%";
	this.categoryList.addEventListener("change", categoryListChange);
	this.popup.appendChild(this.categoryList);
	
	this.list = document.createElement("select");
	this.list.size = 2;
	this.list.style.position = "absolute";
	this.list.style.left = "0";
	this.list.style.top = "50%";
	this.list.style.width = "33%";
	this.list.style.bottom = "0";
	this.list.addEventListener("dblclick", listDoubleClick);
	this.list.addEventListener("change", listChange);
	this.popup.appendChild(this.list);
	
	this.infoContainer = document.createElement("div");
	//this.infoContainer.style.position = "absolute";
	HTMLBoxHelper.setLeftTopRightBottom(this.infoContainer, "absolute", "33%", "24px", "0", "0");
	this.popup.appendChild(this.infoContainer);

	this.infoToolbarContainer = document.createElement("div");
	HTMLBoxHelper.setLeftTop(this.infoToolbarContainer, "absolute", "33%", "24px", "67%", "24px");
	this.infoToolbarContainer.style.backgroundColor = "#eeeeee";
	this.popup.appendChild(this.infoToolbarContainer);

	this.infoToolbar = new ViewToolbar(this.infoToolbarContainer, "small");

	this.info = document.createElement("div");
	HTMLBoxHelper.setLeftTopRightBottom(this.info, "absolute", "33%", "48px", "0", "0");
	this.info.style.overflow = "auto";
	this.popup.appendChild(this.info);
	

	// TODO: show module/project categories as group headers in select
	// TODO: next to the list, show module information details, ViewToolbar as tabs control?
	// TODO: reusable modle information details, want same in "Open..." dialog as well? no, thats different, can show description but not pins an stuff

	this.container.appendChild(this.popup);
	
	Events(this);

	this.modules = [];
}

ModulePopup.prototype.updateModuleInfo = function() {
	var self = this;
	var tabContents = [];

	function infoTabClick(index) {
		// set visibility
		tabContents.forEach(function(tabContent, tabIndex) {
			if (tabIndex == index) {
				tabContent.style.display = "block";
			} else {
				tabContent.style.display = "none";
			}
		});
		
		//self.infoToolbar.highlight();
	}
	
	function bindRequestedModuleInfo(moduleInfo) {
		moduleInfo.tabs.forEach(function(tab, index) {
			self.infoToolbar.createButton(tab.name, "", function() {
				infoTabClick(index);
			});
			
			var details = document.createElement("div");
			details.innerHTML = tab.content;
			tabContents.push(details);
			self.info.appendChild(details);
		});
		infoTabClick(0);
	}
	
	this.info.innerHTML = "";
	this.infoToolbar.container.innerHTML = "";
	this.emit("requestModuleInfo", this.list.value, bindRequestedModuleInfo);//moduleInfo);

}

ModulePopup.prototype.refresh = function() {
	var filterRegex = new RegExp(this.filterTextbox.value, "i");
	var filterCategory = this.categoryList.value;

	var modules = this.modules.filter(function(module) {
		return module.name.search(filterRegex) != -1;
	});

	var categories = getCategories(modules);
	
	HTMLSelectHelper.bindNameValueList(this.categoryList, categories);
	this.categoryList.value = filterCategory;

	modules = modules.filter(function(module) {
		return (filterCategory == "" || module.category == filterCategory);
	});
	
	HTMLSelectHelper.bindNameValueList(this.list, modules);
	
	function getCategories(modules) {
		var categories = {};
		modules.forEach(function(module) {
			if (!categories[module.category]) {
				categories[module.category] = 1;
			} else {
				categories[module.category]++;
			}
		});

		var result = [{name:"(all) (" + modules.length + ")", value:""}];
		for (var categoryName in categories) {
			result.push({name:categoryName + " (" + categories[categoryName].toString() + ")", value:categoryName});
		}
		result.sort(function(a, b) {
			if (a.name < b.name) return -1;
			if (a.name > b.name) return 1;
			return 0;
		});
		return result;
	}
}

ModulePopup.prototype.show = function() {
	this.modules = [];
	this.emit("requestModuleList", this.modules);
	this.refresh();

	this.container.style.display = "block";
	this.filterTextbox.value = "";
	this.filterTextbox.focus();
}

ModulePopup.prototype.hide = function() {
	this.container.style.display = null;
	this.emit("hide");
}
