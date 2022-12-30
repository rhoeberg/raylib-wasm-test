mergeInto(LibraryManager.library, {
	my_js: function() {
		return [2, 1];
	},

	get_window_height: function() {
		return window.innerHeight;
	},
	get_window_width: function() {
		return window.innerWidth;
	},
});
