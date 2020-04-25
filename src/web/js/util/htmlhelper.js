var HTMLBoxHelper = {
	setLeftTop : function(box, position, left, top, width, height) {
		box.style.position = position;
		box.style.left = left;
		box.style.top = top;
		box.style.width = width;
		box.style.height = height;
	},

	setLeftBottom : function(box, position, left, bottom, width, height) {
		box.style.position = position;
		box.style.left = left;
		box.style.bottom = bottom;
		box.style.width = width;
		box.style.height = height;
	},

	setLeftTopRightBottom : function(box, position, left, top, right, bottom) {
		box.style.position = position;
		box.style.left = left;
		box.style.top = top;
		box.style.right = right;
		box.style.bottom = bottom;
	},
	
	setLeftTopWidthBottom : function(box, position, left, top, width, bottom) {
		box.style.position = position;
		box.style.left = left;
		box.style.top = top;
		box.style.width = width;
		box.style.bottom = bottom;
	},
	
	setWidthTopRightBottom : function(box, position, width, top, right, bottom) {
		box.style.position = position;
		box.style.width = width;
		box.style.top = top;
		box.style.right = right;
		box.style.bottom = bottom;
	}
};

var HTMLSelectHelper = {
	bindArray : function(select, list, binder) {
		select.innerHTML = "";

		for (var i = 0; i < list.length; i++) {
			var item = list[i];
			var opt = document.createElement("option");
			binder(opt, item, i);
			select.appendChild(opt);
		}
	},

	bindNameValueList : function(select, list) {
		HTMLSelectHelper.bindArray(select, list, function(opt, item) {
			opt.text = item.name;
			opt.value = item.value;
		});
		/*
		select.innerHTML = "";

		for (var i = 0; i < list.length; i++) {
			var item = list[i];
			var opt = document.createElement("option");
			opt.text = item.name;
			opt.value = item.value;
			select.appendChild(opt);
		}*/
	},

	getOptionByValue : function(options, value) {
		for (var i = 0; i < options.length; i++) {
			var opt = options[i];
			if (opt.value == value) {
				return opt;
			}
		}
		return null;
	},

	clearSelectOptions : function(options) {
		for (var i = 0; i < options.length; i++) {
			var opt = options[i];
			opt.selected = false;
		}
		return null;
	}
};

var HTMLElementHelper = {
	// http://stackoverflow.com/questions/8111094/cross-browser-javascript-function-to-find-actual-position-of-an-element-in-page
	offset : function(element) {
		var body = document.body,
			win = document.defaultView,
			docElem = document.documentElement,
			box = document.createElement('div');
		box.style.paddingLeft = box.style.width = "1px";
		body.appendChild(box);
		var isBoxModel = box.offsetWidth == 2;
		body.removeChild(box);
		box = element.getBoundingClientRect();
		var clientTop  = docElem.clientTop  || body.clientTop  || 0,
			clientLeft = docElem.clientLeft || body.clientLeft || 0,
			scrollTop  = win.pageYOffset || isBoxModel && docElem.scrollTop  || body.scrollTop,
			scrollLeft = win.pageXOffset || isBoxModel && docElem.scrollLeft || body.scrollLeft;
		return {
			top : box.top  + scrollTop  - clientTop,
			left: box.left + scrollLeft - clientLeft};
	}
};

// based on http://stackoverflow.com/questions/901115/how-can-i-get-query-string-values-in-javascript
var QueryString = {
	parameters : (
		function(a) {
			if (a == "") return {};
			var b = {};
			for (var i = 0; i < a.length; ++i)
			{
				var p=a[i].split('=', 2);
				if (p.length == 1)
					b[p[0]] = "";
				else
					b[p[0]] = decodeURIComponent(p[1].replace(/\+/g, " "));
			}
			return b;
		})(window.location.search.substr(1).split('&'))
};

// https://mathiasbynens.be/notes/xhr-responsetype-json
// http://stackoverflow.com/questions/6418220/javascript-send-json-object-with-ajax
var Ajax = { 
	getJSON : function(url, successHandler, errorHandler) {
		var xhr = new XMLHttpRequest();
		xhr.open('get', url, true);
		xhr.responseType = 'json';
		xhr.onload = function() {
			var status = xhr.status;
			if (status >= 200 && status <= 299) {
				successHandler && successHandler(xhr.response);
			} else {
				errorHandler && errorHandler(status);
			}
		};
		xhr.send();
	},
	sendJSON : function(method, url, data, successHandler, errorHandler) {
		var xhr = new XMLHttpRequest();
		xhr.open(method, url, true);
		xhr.responseType = 'json';
		xhr.onload = function() {
			var status = xhr.status;
			if (status >= 200 && status <= 299) {
				successHandler && successHandler(xhr.response);
			} else {
				errorHandler && errorHandler(status);
			}
		};
		xhr.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
		xhr.send(JSON.stringify(data));
		//xhr.send();
	}
};
