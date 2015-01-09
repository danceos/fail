$.ajaxSetup({
async: false
});


//Umwandlung von dezimal zu hexadezimal
function dec2hex(dec) {
	var hex = "";

	if (dec < 0) {
		dec = 0xFFFFFFFF + dec + 1;
	}

	hex = parseInt(dec, 10).toString(16);

	return hex;
}

//Berechne den Farbwert (z.B.: #FFFFFF) fuer gegebene Fehlerwerte
function calcColor(fehler, maxFehler) {
	//console.log("Farbe Fehler: " + fehler + " Max: " + maxFehler);

	var part = 0;

	if (fehler != 0) {
		var x =  255/Math.log(maxFehler);
		part = x*Math.log(fehler);
	}

	//console.log("Farbe Fehler: " + fehler + " Max: " + maxFehler + " Part: " + part);

	var brightness = 255 - part;
	var hex = dec2hex(brightness);

	var colorcode;

	if (brightness <= 0xf) {
		colorcode = "#FF" + "0" + hex + "0" + hex;
	} else {
		colorcode = "#FF" + hex + hex;
	}
	//console.log("Farbe Fehler: " + fehler + " Max: " + maxFehler + " Part: " + part + " Farbcode: " + colorcode);
	return colorcode;
}

//Tooltip für Instruktionszeilen mit Fehler
$('body').popover({
	selector: ".hasFehler",
	html: true,
	title: function() { return '0x' + $(this).attr("data-address"); },
	content: function() {
		var ausgabe = '';
		var caller = $(this);

		$.getJSON("core.php", {cmd: 'getResultTypes'}, function(data) {
			$.each(data, function(key, val) {
				ausgabe = ausgabe  + val +': ' + caller.attr(val) + '<br>';
			});
		});
		return ausgabe;
	}
});

//Einblenden des Modal "Loading" fuer Ajax Requests
/*$(document).ajaxStart(function(){
	//console.log("ajaxstart");
	$("body").addClass("loading");
}).ajaxStop(function(){
	//console.log("ajaxstop");
	$("body").removeClass("loading");
});*/

//User-Interaktionen nachdem das DOM geladen ist
$(document).ready(function() {

	//DB check
	$.getJSON("core.php", {cmd: 'dbTest'}, function(data) {

		if(data != "ok") {
			$('html').html(data);
		}
	});

	//Holen der Binarys, die in der DB in der Tabelle Variant vorkommen
	$.getJSON("core.php", {cmd: 'getBinarys'}, function(data) {
		$.each(data, function(key, val) {
			$('#binary').append('<option value="' + val + '">' + val + '</option>');
		});
	});

	//Auswahl eines Eintrages aus den Source-Files für die Darstellung des Hochsprachencode
	$('#sourceFiles').change(function() {
		if($(this).val() != 'none') {
			$.getJSON("core.php", {cmd: 'getHighlevelCode', variant_id: $('#variant').val(), file_id: $(this).val(), version: $('#faerbung').val()}, function(data) {

				$('#hcode').html(data);

				$('.maxFehlerMapping').on( "calcColor", function( event, newMaxFehler, activeFehlertypes) {
									var newFehler = 0;
									var actualRow = $(this);

									$.each(activeFehlertypes, function(key, val) {
										newFehler = newFehler + parseInt(actualRow.attr(val));
									});

									// console.log("jepp" + newMaxFehler + " " + newFehler);

									$(this).prev().prev().prev().css("background-color", calcColor(newFehler, newMaxFehler))
										   .css('cursor', 'pointer');
								});

				$('.hasFehler').on( "calcColor", function( event, newMaxFehler, activeFehlertypes) {
									var newFehler = 0;
									var actualRow = $(this);

									$.each(activeFehlertypes, function(key, val) {
										newFehler = newFehler + parseInt(actualRow.attr(val));
									});

									$(this).css("background-color", calcColor(newFehler, newMaxFehler))
										   .css('cursor', 'pointer');
								});
			});


			setTimeout(function(){
				var activeFehlertypes = new Array();
				var newMaxFehler = 0;

				$('#fehlertypenset > .active').each(function(){
					activeFehlertypes.push($(this).attr("id"));
					//console.log("Hinzugefuegt: " + $(this).attr("id"));
				});


				$.each(activeFehlertypes,function(key, name){

					//Neuen MaxFehler berechnen
					newMaxFehler = newMaxFehler + parseInt($('#maxFehler').attr(name));
				});
				$('.hasFehler').trigger('calcColor', [ newMaxFehler, activeFehlertypes ]);
				$('.maxFehlerMapping').trigger('calcColor', [ newMaxFehler, activeFehlertypes ]);
			},100);
		}
	});

	//Auswahl eines Eintrages aus den Binarys für die Darstellung des Assembler-Code
	$('#binary').change(function() {
		if($(this).val() != 'none') {
			$('#variant').html('<option value="none" selected="selected"></option>');
			$.getJSON("core.php", {cmd: 'getVariants', datei: $(this).val()}, function(data) {
				$.each(data, function(key, val) {
					$('#variant').append('<option value="' + key + '">' + val + '</option>');
				});
			});
		}
	});

	//Analyse Button wird gedrueckt
	$('#analyse').button().click( function () {

		$.getJSON("core.php", {cmd: 'getAsmCode', variant_id: $('#variant').val(), version: $('#faerbung').val()}, function(data) {
			$('#asm').html(data);
		});

		$.getJSON("core.php", {cmd: 'getSourceFiles', variant_id: $('#variant').val()}, function(data) {
			$.each(data, function(key, val) {
				$('#sourceFiles').append('<option value="' + key + '">' + val + '</option>');
			});
		});

		$.getJSON("core.php", {cmd: 'getResultTypes'}, function(data) {

			$('#fehlertypenset').html('');
			$.each(data, function(key, val) {
				$('#fehlertypenset').append('<label class="btn btn-default" id="' + val + '"><input type="checkbox" id="' + val + '">' + val + '</label>');
				//$('#'+ val +'.btn').button('toggle');
			});

			$('.hasFehler').on( "calcColor", function( event, newMaxFehler, activeFehlertypes) {
									var newFehler = 0;
									var actualRow = $(this);

									$.each(activeFehlertypes, function(key, val) {
										newFehler = newFehler + parseInt(actualRow.attr(val));
									});

									$(this).css("background-color", calcColor(newFehler, newMaxFehler))
										   .css('cursor', 'pointer');
								});

			//Auf Änderungen bzgl. des Fehler-Buttonset horchen
			$('#fehlertypenset input[type=checkbox]').change(function() {setTimeout(function(){
				//console.log("aenderung!");
				var activeFehlertypes = new Array();
				var newMaxFehler = 0;

				$('#fehlertypenset > .active').each(function(){
					activeFehlertypes.push($(this).attr("id"));
				});


				$.each(activeFehlertypes,function(key, name){
					//Neuen MaxFehler berechnen
					newMaxFehler = newMaxFehler + parseInt($('#maxFehler').attr(name));
				});

				$('.hasFehler').trigger('calcColor', [ newMaxFehler, activeFehlertypes ]);
				$('.maxFehlerMapping').trigger('calcColor', [ newMaxFehler, activeFehlertypes ]);
			},100)});
		});
	});
});
