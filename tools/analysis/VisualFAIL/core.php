<?php
require('CONFIGURATION.php');

// increase default script runtime limit
set_time_limit(60*10);

//Datenbankverbindung aufbauen
$verbindung = mysql_connect ($host,$username, $password)
					or die ("MySQL connection failed.");

mysql_select_db($database) or die ("Cannot select database '$database'.");

// identify command
switch ($_GET['cmd']) {
	case "dbTest"			: dbTest();break;
	case "getAsmCode"		: getAsmCode();break;
	case "asmToSourceFile"	: asmToSourceFile();break;
	case "getBinarys"		: getBinarys();break;
	case "getVariants"		: getVariants();break;
	case "getSourceFiles"	: getSourceFiles();break;
	case "getResultTypes"		: getResulttypesOUT();break;
	case "getHighlevelCode"	: getHighlevelCode();break;
	case "resultsDB"		: resultsDB();break;
	case "dechex"			: echo json_encode(dechex($_GET['dec']));break;
}

mysql_close($verbindung);

function dbTest()
{

	$check = true;

	$abfrage = "SELECT 1 FROM objdump;";

	$ergebnis = mysql_query($abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle objdump nicht gefunden <br>');
		return;
	}

	$abfrage = "SELECT 1 FROM fulltrace;";

	$ergebnis = mysql_query($abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle fulltrace nicht gefunden <br>');
		return;
	}

	$abfrage = "SELECT 1 FROM dbg_filename;";

	$ergebnis = mysql_query($abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_filename nicht gefunden <br>');
		return;
	}

	$abfrage = "SELECT 1 FROM dbg_mapping;";

	$ergebnis = mysql_query($abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_mapping nicht gefunden <br>');
		return;
	}

/*
	$abfrage = "SELECT 1 FROM dbg_methods;";

	$ergebnis = mysql_query($abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_methods nicht gefunden <br>');
		return;
	}
*/

	$abfrage = "SELECT 1 FROM dbg_source;";

	$ergebnis = mysql_query($abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_source nicht gefunden <br>');
		return;
	}

/*
	$abfrage = "SELECT 1 FROM dbg_stacktrace;";

	$ergebnis = mysql_query($abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_stacktrace nicht gefunden <br>');
		return;
	}

	$abfrage = "SELECT 1 FROM dbg_variables;";

	$ergebnis = mysql_query($abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_variables nicht gefunden <br>');
		return;
	}
*/
}

function getBinarys()
{
	$binarys = array();

	$abfrage = "SELECT benchmark FROM variant;";

	$ergebnis = mysql_query($abfrage);

	while ($row = mysql_fetch_object($ergebnis)) {
		array_push($binarys, $row->benchmark);
	}

	$result = array_unique($binarys);

	echo json_encode($result);
}

function getVariants()
{
	$variants = array();

	$abfrage = "SELECT id, variant FROM variant WHERE benchmark = '" . $_GET['datei'] ."';";

	$ergebnis = mysql_query($abfrage);

	while ($row = mysql_fetch_object($ergebnis)) {
		$variants[$row->id] = $row->variant;
	}

	echo json_encode($variants);
}

function getSourceFiles()
{
	$sourceFiles = array();

	$abfrage = "SELECT file_id, path FROM dbg_filename WHERE variant_id = '" . $_GET['variant_id']. "';";

	$ergebnis = mysql_query($abfrage);

	while ($row = mysql_fetch_object($ergebnis)) {
		$sourceFiles[$row->file_id] = $row->path;
	}

	echo json_encode($sourceFiles);
}

function asmCode()
{
	$content = "";

	$abfrage = "SELECT instr_address, disassemble FROM objdump WHERE variant_id = '" . $_GET['variant_id'] ."' ORDER BY instr_address;";

	$ergebnis = mysql_query($abfrage);

	$content = $content;
	while ($row = mysql_fetch_object($ergebnis)) {
		$content .= '<span id="' . dechex($row->instr_address) . '">' . dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '</span><br>';
	}
	echo json_encode($content);
}

function getAsmCode()
{
	$content = "";
	$resulttypes = array();

	$asmAbfrage = "SELECT instr_address, disassemble FROM objdump WHERE variant_id = '" . $_GET['variant_id'] ."' ORDER BY instr_address;";

	$asmcode = mysql_query($asmAbfrage);

	getResulttypes($resulttypes);

	$fehlerdaten = resultsDB($_GET['variant_id'], $_GET['version'], $resulttypes);
	//$fehlerdaten = askDBFehler($_GET['variant_id'], $resulttypes, $_GET['version']);

	//print_r($fehlerdaten);
	$content = '<div id="maxFehler" ';
	foreach ($resulttypes as $value) {
        $temp = $value . '="' . $fehlerdaten['max'][$value] . '" ';
        $content .= $temp;
    }
	$content .= ' >';
	while ($row = mysql_fetch_object($asmcode)) {
		if (array_key_exists($row->instr_address,$fehlerdaten['Daten'])) {
			$content .= '<span id="' . dechex($row->instr_address) . '" class="hasFehler" ';

			foreach ($resulttypes as $value) {
				$content .= $value . '="' . $fehlerdaten['Daten'][$row->instr_address][$value] . '" ';
			}

			$content .= ' cursor: pointer;>' . dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '</span><br>';
		} else {
			$content .= '<span id="' . dechex($row->instr_address) . '">' . dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '</span><br>';
		}
	}

	$content .= ' </div>';

	echo json_encode($content);
}

function getHighlevelCode()
{
	$content = "";
	$resulttypes = array();

	getResulttypes($resulttypes);

	$kleinsteAdresseAbfrage = "SELECT MIN(instr_address) AS instr_address FROM objdump WHERE variant_id = " . $_GET['variant_id'];
	$kleinsteAdresseErgebnis = mysql_query($kleinsteAdresseAbfrage) or trigger_error($kleinsteAdresseAbfrage, E_USER_NOTICE);
	$kleinsteAdresse = mysql_fetch_object($kleinsteAdresseErgebnis);

	$highlevelCodeAbfrage = "SELECT linenumber, line FROM dbg_source WHERE variant_id = '" . $_GET['variant_id']. "' AND file_id = '" . $_GET['file_id']. "' ORDER BY linenumber;";
	$mappingAbfrage = "SELECT instr_absolute, linenumber FROM dbg_mapping WHERE variant_id = '" . $_GET['variant_id']. "' AND file_id = '" . $_GET['file_id'] . "' AND instr_absolute >= '" . $kleinsteAdresse->instr_address . "' ORDER BY instr_absolute;";

	$highlevelCode = mysql_query($highlevelCodeAbfrage);
	$mappingInfo = mysql_query($mappingAbfrage);

	$fehlerdaten = resultsDB($_GET['variant_id'], $_GET['version'], $resulttypes);

	$mappingRanges = array();
	$numEntrysMapping = mysql_num_rows($mappingInfo);

	$row = mysql_fetch_object($mappingInfo);

	// FIXME use values from DB instead
	for ($i = 0; $i < $numEntrysMapping-1; $i++) {
		if (!is_array($mappingRanges[$row->linenumber])) {
			$mappingRanges[$row->linenumber] = array();
		}
		$oldLineNumber = $row->linenumber;
		$firstAddr = $row->instr_absolute;
		$row = mysql_fetch_object($mappingInfo);
		array_push($mappingRanges[$oldLineNumber], array($firstAddr, $row->instr_absolute));
	}

	$mapping = array();
	$maxFehlerMapping = array();

	foreach ($mappingRanges as $lineNumber => $value) {
		$maxFehler = array();
		foreach ($resulttypes as $val) {
			$maxFehler[$val] = 0;
		}
		foreach ($value as $index => $ranges) {
				// was ">" instead of ">=" before
				$InstrMappingAbfrage = "SELECT instr_address, disassemble FROM objdump WHERE variant_id = '" . $_GET['variant_id']. "' AND instr_address >= '" . $ranges[0] . "' AND instr_address < '" . $ranges[1] . "' ORDER BY instr_address;";
				$mappingErgebnis = mysql_query($InstrMappingAbfrage);
				//Leerzeile
				if (mysql_num_rows($mappingErgebnis) > 0) {
					$mapping[$lineNumber] [] = '<br>';
				}
				while ($row = mysql_fetch_object($mappingErgebnis)) {

					if (array_key_exists($row->instr_address,$fehlerdaten['Daten'])) {
						$newline .= '<span id="' . dechex($row->instr_address) . '" class="hasFehler" ';

						foreach ($resulttypes as $value) {
							$newline .= $value . '="' . $fehlerdaten['Daten'][$row->instr_address][$value] . '" ';
							$maxFehler[$value] = $maxFehler[$value] + $fehlerdaten['Daten'][$row->instr_address][$value];
						}

						$newline .= ' cursor: pointer;>' . dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '</span><br>';
					} else {
						$newline = '<span id="' . dechex($row->instr_address) . '">' . dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '</span><br>';
					}
					$mapping[$lineNumber] [] = $newline;
				}

				//Leerzeile
				//$mapping[$lineNumber] [] = '<br>';
		}
		foreach ($resulttypes as $value) {
			$maxFehlerMapping[$lineNumber][$value] = $maxFehler[$value];
		}
	}

	while ($row = mysql_fetch_object($highlevelCode)) {
		$content .= '<span id="' . $row->linenumber . '">' . $row->linenumber . ' : ' . $row->line . '</span><br>';
		if (array_key_exists($row->linenumber, $mapping)) {
			$content .= '<div id="mapping">';
			foreach ($mapping[$row->linenumber] as $index => $span) {
				$content .= $span;
			}
			$content .= '</div>';
			$content .= '<div class="maxFehlerMapping"';
			foreach ($resulttypes as $value) {
					$content .= $value . '="' . $maxFehlerMapping[$row->linenumber][$value] . '" ';
			}
			$content .= '> </div>';
		}
	}

	echo json_encode($content);
}

function getResulttypes(&$resulttypes)
{
	$abfrage = "SELECT resulttype FROM " . $GLOBALS['result_table'] . " GROUP BY resulttype;";

	$ergebnis = mysql_query($abfrage);

	while ($row = mysql_fetch_object($ergebnis)) {
		//echo $row->resulttype;
		array_push($resulttypes, $row->resulttype);
	}
}

function getResulttypesOUT()
{
	$resulttypes = array();

	$abfrage = "SELECT resulttype FROM " . $GLOBALS['result_table'] . " GROUP BY resulttype;";

	$ergebnis = mysql_query($abfrage);

	while ($row = mysql_fetch_object($ergebnis)) {
		//echo $row->resulttype;
		array_push($resulttypes, $row->resulttype);
	}

	echo json_encode($resulttypes);
}

function askDBFehler($variant_id, $resulttypes, $version)
{
	if ($version == 'onlyRightEdge') {
		// we don't need fulltrace here at all
		$abfrage = "SELECT t.instr2 AS instr, t.instr2_absolute AS instr_absolute";
		foreach ( $resulttypes as $value) {
			$abfrage .= ", SUM(IF(r.resulttype = '" . $value . "', 1, 0)*(t.time2-t.time1+1)) AS " . $value;
		}
		$abfrage .= " FROM trace t
			JOIN fsppilot p
			ON t.variant_id = p.variant_id
			AND t.data_address = p.data_address
			AND p.instr2 = t.instr2
			JOIN " . $GLOBALS['result_table'] . " r
			ON p.id = r.pilot_id
			WHERE t.variant_id = $variant_id
			AND t.accesstype = 'R'
			GROUP BY t.instr2_absolute;";
	} else if ($version == 'latestip') {
		$abfrage = "SELECT r.latest_ip ";
		foreach ( $resulttypes as $value) {
			$abfrage .= ", SUM(IF(r.resulttype = '" . $value . "', 1, 0)*(t.time2-t.time1+1)) AS " . $value;
		}
		$abfrage .= " FROM trace t
								JOIN fsppilot p
								  ON t.variant_id = p.variant_id
								  AND t.data_address = p.data_address
								  AND p.instr2 = t.instr2
								JOIN " . $GLOBALS['result_table'] . " r
								  ON p.id = r.pilot_id
								WHERE t.variant_id = '" . $variant_id . "' AND t.accesstype = 'R'
								GROUP BY r.latest_ip;";
	} else {
		$abfrage = "SELECT ft.instr, ft.instr_absolute ";
					foreach ( $resulttypes as $value) {
						$abfrage .= ", SUM(IF(r.resulttype = '" . $value . "', 1, 0)*(t.time2-t.time1+1)) AS " . $value;
					}
		$abfrage .= " FROM fulltrace ft
								LEFT JOIN trace t
								  ON ft.variant_id = '" . $variant_id . "'
								 AND t.variant_id = '" . $variant_id . "'
								 AND ft.instr BETWEEN t.instr1 AND t.instr2
								 AND t.accesstype = 'R'
								JOIN fsppilot p
								  ON t.variant_id = '" . $variant_id . "'
								  AND p.variant_id = '" . $variant_id . "'
								  AND t.data_address = p.data_address
								  AND p.instr2 = t.instr2
								JOIN " . $GLOBALS['result_table'] . " r
								  ON p.id = r.pilot_id
								GROUP BY ft.instr_absolute;";
	}

	//echo $abfrage;

	$ergebnis = mysql_query($abfrage);

	return $ergebnis;
}

function resultsDB($variant_id, $version, $resulttypes)
{

	getResulttypes($resulttypes);

	//print_r($resulttypes);

	$ergebnis = askDBFehler($variant_id, $resulttypes, $version);

	//print_r($ergebnis);

	$results = array();

	// We find the fields number
	$numfields=mysql_num_fields($ergebnis);

	for ($i=0; $i < $numfields; $i++) {
		$fieldname[$i]=mysql_field_name($ergebnis, $i);
	}

	for ($i=2; $i < $numfields; $i++) {
		$results["max"][$fieldname[$i]] = 0;
	}

	$maxFehler = 0;
	while ($row = mysql_fetch_object($ergebnis)) {
		if ($version != 'latestip') {
			if ($row->instr_absolute != NULL) {
				$results["Daten"][$row->instr_absolute] = array();
				for ($i = 2; $i < $numfields ; $i++) {
					$results["Daten"][$row->instr_absolute][$fieldname[$i]] = $row->$fieldname[$i];

					if ($row->$fieldname[$i] > $results["max"][$fieldname[$i]]) {
						$results["max"][$fieldname[$i]] = $row->$fieldname[$i];
					}
				}
			}
		} else {
			if ($row->latest_ip != NULL) {
				$results["Daten"][$row->latest_ip] = array();
				for ($i = 0 ; $i < $numfields ; $i++) {
					$results["Daten"][$row->latest_ip][$fieldname[$i]] = $row->$fieldname[$i];

					if ($row->$fieldname[$i] > $results["max"][$fieldname[$i]]) {
						$results["max"][$fieldname[$i]] = $row->$fieldname[$i];
					}
				}
			}
		}
	}

	return $results;
}

?>
