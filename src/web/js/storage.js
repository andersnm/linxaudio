function LinxStorage(makeNew) {
	var self = this;

	var created = false;
	
	function onerror(event) {
		console.log("Error loading database");
	}
	
	function onsuccess(event) {
		self.db = event.target.result;
		self.emit("ready", created);
	}
	
	function onupgradeneeded(event) {
		console.log("Creating new project database, old version = " + event.oldVersion);
		var db = event.target.result;
		db.onerror = onerror; 

		if (event.oldVersion < 1) {
			var pluginProjectStore = db.createObjectStore("PluginProject", { keyPath: "system.product" });
			pluginProjectStore.createIndex("author", "system.author", { unique: false });
			
			var songProjectStore = db.createObjectStore("SongProject", { keyPath: "name" });

			created = true;
		} else {
			console.log("Unexpected old version");
		}
		
	}


	this.db = null;
	
	if (makeNew) {
		window.indexedDB.deleteDatabase("ProjectStorage");
	}
	
	//window.indexedDB.deleteDatabase("LinxAudioGraphDefinition");
	//var openRequest = window.indexedDB.open("LinxAudioGraphDefinition", 1);
	var openRequest = window.indexedDB.open("ProjectStorage", 1);
	openRequest.onerror = onerror;
	openRequest.onsuccess = onsuccess;
	openRequest.onupgradeneeded = onupgradeneeded;
	
	Events(this);
}

LinxStorage.prototype.deleteDatabase = function() {

	function onsuccess() {
		console.log("database deleted");
	}

	function onerror() {
		console.log("database delete error");
	}

	function onblocked() {
		console.log("database delete blocked");
	}

	if (this.db) {
		this.db.close();
		this.db = null;
	}

	var deleteRequest = window.indexedDB.deleteDatabase("ProjectStorage");
	deleteRequest.onsuccess = onsuccess;
	deleteRequest.onerror = onerror;
	deleteRequest.onblocked = onblocked;	
}

LinxStorage.prototype.savePluginProject = function(objectProject) {
	function onSaveRequestSuccess() {
		console.log("put success");
	}
	
	var transaction = this.db.transaction(["PluginProject"], "readwrite");	
	var projectStore = transaction.objectStore("PluginProject");
	var saveRequest = projectStore.put(objectProject);
	saveRequest.onsuccess = onSaveRequestSuccess;
}

LinxStorage.prototype.getPluginProjectHeaders = function(callback) {	
	var result = [];
	function onCursorSuccess(e) {
		var cursor = e.target.result;
		if (cursor) {
			result.push({product : cursor.value.system.product, author : cursor.value.system.author });
			cursor.continue();
		} else {
			if (callback) callback(result);
		}
	}
	
	var transaction = this.db.transaction(["PluginProject"], "readonly");
	var projectStore = transaction.objectStore("PluginProject");
	var projectCursor = projectStore.openCursor();
	projectCursor.onsuccess = onCursorSuccess ;
}

LinxStorage.prototype.getPluginProject = function(name, callback) {		
	function onGetSuccess(e) {
		if (callback) callback(e.target.result);
	}
	
	var transaction = this.db.transaction(["PluginProject"], "readonly");
	var projectStore = transaction.objectStore("PluginProject");
	var request = projectStore.get(name);
	request.onsuccess = onGetSuccess;
}


LinxStorage.prototype.saveSongProject = function(objectProject) {
	function onSaveRequestSuccess() {
		console.log("put success");
	}
	
	var transaction = this.db.transaction(["SongProject"], "readwrite");	
	var projectStore = transaction.objectStore("SongProject");
	var saveRequest = projectStore.put(objectProject);
	saveRequest.onsuccess = onSaveRequestSuccess;
}

LinxStorage.prototype.getSongProjectHeaders = function(callback) {	
	var result = [];
	function onCursorSuccess(e) {
		var cursor = e.target.result;
		if (cursor) {
			result.push({name : cursor.value.name });
			cursor.continue();
		} else {
			if (callback) callback(result);
		}
	}
	
	var transaction = this.db.transaction(["SongProject"], "readonly");
	var projectStore = transaction.objectStore("SongProject");
	var projectCursor = projectStore.openCursor();
	projectCursor.onsuccess = onCursorSuccess ;
}

LinxStorage.prototype.getSongProject = function(name, callback) {		
	function onGetSuccess(e) {
		if (callback) callback(e.target.result);
	}
	
	var transaction = this.db.transaction(["SongProject"], "readonly");
	var projectStore = transaction.objectStore("SongProject");
	var request = projectStore.get(name);
	request.onsuccess = onGetSuccess;
}
