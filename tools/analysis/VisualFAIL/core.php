<?php
require('CONFIGURATION.php');

// increase default script runtime limit
set_time_limit(60*10);

//Datenbankverbindung aufbauen
$db = mysqli_connect ($host,$username, $password)
					or die ("MySQL connection failed.");

mysqli_select_db($db, $database) or die ("Cannot select database '$database'.");

// identify command
switch ($_GET['cmd']) {
	case "dbTest"			: dbTest();break;
	case "getAsmCode"		: getAsmCode();break;
	case "asmToSourceFile"	: asmToSourceFile();break;
	case "getBinarys"		: getBinarys();break;
	case "getVariants"		: getVariants();break;
	case "getSourceFiles"	: getSourceFiles();break;
	case "getResultTypes"	: getResulttypesOUT();break;
	case "getHighlevelCode"	: getHighlevelCode();break;
	case "resultsDB"		: resultsDB();break;
	case "dechex"			: echo json_encode(dechex($_GET['dec']));break;
}

mysqli_close($db);

function dbTest()
{

	$check = true;

	$abfrage = "SELECT 1 FROM objdump;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle objdump nicht gefunden <br>');
		return;
	}

	$abfrage = "SELECT 1 FROM fulltrace;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle fulltrace nicht gefunden <br>');
		return;
	}

	$abfrage = "SELECT 1 FROM dbg_filename;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_filename nicht gefunden <br>');
		return;
	}

	$abfrage = "SELECT 1 FROM dbg_mapping;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_mapping nicht gefunden <br>');
		return;
	}

/*
	$abfrage = "SELECT 1 FROM dbg_methods;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_methods nicht gefunden <br>');
		return;
	}
*/

	$abfrage = "SELECT 1 FROM dbg_source;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_source nicht gefunden <br>');
		return;
	}

/*
	$abfrage = "SELECT 1 FROM dbg_stacktrace;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	if (!$ergebnis) {
		echo json_encode('Tabelle dbg_stacktrace nicht gefunden <br>');
		return;
	}

	$abfrage = "SELECT 1 FROM dbg_variables;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

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

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	while ($row = mysqli_fetch_object($ergebnis)) {
		array_push($binarys, $row->benchmark);
	}

	$result = array_unique($binarys);

	echo json_encode($result);
}

function getVariants()
{
	$variants = array();

	$abfrage = "SELECT id, variant FROM variant WHERE benchmark = '" . $_GET['datei'] ."';";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	while ($row = mysqli_fetch_object($ergebnis)) {
		$variants[$row->id] = $row->variant;
	}

	echo json_encode($variants);
}

function remove_common_prefix($files)
{
	if (count($files) == 0) {
		return array();
	}

	// start with arbitrary file
	$commonprefix = dirname(current($files));

	foreach ($files AS $id => $file) {
		for ($i = 0; $i < strlen($commonprefix); ++$i) {
			if ($i >= strlen($file) || $file[$i] != $commonprefix[$i]) {
				$commonprefix = substr($commonprefix, 0, $i);
				break;
			}
		}
		if (strlen($commonprefix) == 0) {
			break;
		}
	}

	$out = array();
	foreach ($files AS $id => $file) {
		$out[$id] = substr($file, strlen($commonprefix));
	}

	return $out;
}

function getSourceFiles()
{
	$sourceFiles = array();

	$abfrage = "SELECT file_id, path FROM dbg_filename WHERE variant_id = '" . $_GET['variant_id']. "';";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	while ($row = mysqli_fetch_object($ergebnis)) {
		$sourceFiles[$row->file_id] = $row->path;
	}

	$sourceFiles = remove_common_prefix($sourceFiles);

	echo json_encode($sourceFiles);
}

function asmCode()
{
	$content = "";

	$abfrage = "SELECT instr_address, disassemble FROM objdump WHERE variant_id = '" . $_GET['variant_id'] ."' ORDER BY instr_address;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	$content = $content;
	while ($row = mysqli_fetch_object($ergebnis)) {
		$content .= dechex($row->instr_address) . '<br>';
	}
	echo json_encode($content);
}

function collapse_repeated($html, $disasm, $force_finish)
{
	static $last_disasm = '';
	static $collect = array();
	$limit_before = $limit_after = 3;

	$out = '';
	if ($force_finish || $last_disasm != $disasm) {
		if (count($collect) > $limit_before + $limit_after + 1) {
			for ($i = 0; $i < $limit_before; ++$i) {
				$out .= $collect[$i];
			}
			$out .= '<i>-- omitted ' . (count($collect) - $limit_before - $limit_after) . " repetitions of '$last_disasm'</i><br>";
			for ($i = count($collect) - $limit_after; $i < count($collect); ++$i) {
				$out .= $collect[$i];
			}
		} else {
			$out = implode('', $collect);
		}
		$last_disasm = $disasm;
		$collect = array();
	}

	if ($force_finish) {
		$out .= $html;
	} else {
		$collect[] = $html;
	}

	return $out;
}

function getAsmCode()
{
	$content = "";
	$resulttypes = array();

	$asmAbfrage = "SELECT instr_address, disassemble FROM objdump WHERE variant_id = '" . $_GET['variant_id'] ."' ORDER BY instr_address;";

	$asmcode = mysqli_query($GLOBALS['db'], $asmAbfrage);

	getResulttypes($resulttypes);

	$fehlerdaten = resultsDB($_GET['variant_id'], $_GET['version'], $resulttypes);
	//$fehlerdaten = askDBFehler($_GET['variant_id'], $resulttypes, $_GET['version']);

	//print_r($fehlerdaten);
	// FIXME id not unique
	$content = '<div id="maxFehler" ';
	foreach ($resulttypes as $value) {
		$temp = $value . '="' . $fehlerdaten['max'][$value] . '" ';
		$content .= $temp;
	}
	$content .= ' >';
	while ($row = mysqli_fetch_object($asmcode)) {
		if (array_key_exists($row->instr_address,$fehlerdaten['Daten'])) {
			$line = '<span data-address="' . dechex($row->instr_address) . '" class="hasFehler" ';

			foreach ($resulttypes as $value) {
				$line .= $value . '="' . $fehlerdaten['Daten'][$row->instr_address][$value] . '" ';
			}

			$line .= ' style="cursor: pointer;">' . dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '</span><br>';
			$content .= collapse_repeated($line, 'dontcare', true);
		} else {
			$line = dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '<br>';
			$content .= collapse_repeated($line, htmlspecialchars($row->disassemble), false);
		}
	}
	$content .= collapse_repeated('', '', true);
	$content .= ' </div>';

	echo json_encode($content);
}

function getHighlevelCode()
{
	$content = "";
	$resulttypes = array();

	getResulttypes($resulttypes);

	$kleinsteAdresseAbfrage = "SELECT MIN(instr_address) AS instr_address FROM objdump WHERE variant_id = " . $_GET['variant_id'];
	$kleinsteAdresseErgebnis = mysqli_query($GLOBALS['db'], $kleinsteAdresseAbfrage) or trigger_error($kleinsteAdresseAbfrage, E_USER_NOTICE);
	$kleinsteAdresse = mysqli_fetch_object($kleinsteAdresseErgebnis);

	$highlevelCodeAbfrage = "SELECT linenumber, line FROM dbg_source WHERE variant_id = '" . $_GET['variant_id']. "' AND file_id = '" . $_GET['file_id']. "' ORDER BY linenumber;";
	$mappingAbfrage = "SELECT linenumber, instr_absolute, line_range_size FROM dbg_mapping WHERE variant_id = '" . $_GET['variant_id']. "' AND file_id = '" . $_GET['file_id'] . "' AND instr_absolute >= '" . $kleinsteAdresse->instr_address . "' ORDER BY instr_absolute;";

	$highlevelCode = mysqli_query($GLOBALS['db'], $highlevelCodeAbfrage);
	$mappingInfo = mysqli_query($GLOBALS['db'], $mappingAbfrage);

	$fehlerdaten = resultsDB($_GET['variant_id'], $_GET['version'], $resulttypes);

	// retrieve mapping of linenumber -> array of [start-address;end-address) ranges
	$mappingRanges = array();
	while (($row = mysqli_fetch_object($mappingInfo))) {
		if (!isset($mappingRanges[$row->linenumber])) {
			$mappingRanges[$row->linenumber] = array();
		}
		$mappingRanges[$row->linenumber][] =
			array(intval($row->instr_absolute),
				$row->instr_absolute + $row->line_range_size);
	}

	$mapping = array();
	// "maxFehler" should be "sumFehler" or alike
	$maxFehlerMapping = array();

	foreach ($mappingRanges as $lineNumber => $value) {
		$maxFehler = array();
		foreach ($resulttypes as $val) {
			$maxFehler[$val] = 0;
		}
		foreach ($value as $index => $ranges) {
			// was ">" instead of ">=" before
			$InstrMappingAbfrage = "SELECT instr_address, disassemble FROM objdump WHERE variant_id = '" . $_GET['variant_id']. "' AND instr_address >= '" . $ranges[0] . "' AND instr_address < '" . $ranges[1] . "' ORDER BY instr_address;";
			$mappingErgebnis = mysqli_query($GLOBALS['db'], $InstrMappingAbfrage);
			while ($row = mysqli_fetch_object($mappingErgebnis)) {
				if (array_key_exists($row->instr_address,$fehlerdaten['Daten'])) {
					$newline = '<span data-address="' . dechex($row->instr_address) . '" class="hasFehler" ';

					foreach ($resulttypes as $value) {
						// FIXME prefix with 'data-results-', adapt JS
						$newline .= $value . '="' . $fehlerdaten['Daten'][$row->instr_address][$value] . '" ';
						$maxFehler[$value] += $fehlerdaten['Daten'][$row->instr_address][$value];
					}

					$newline .= ' style="cursor: pointer;">' . dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '</span><br>';
					$newline = collapse_repeated($newline, 'dontcare', true);
				} else {
					$newline = dechex($row->instr_address) . '     ' . htmlspecialchars($row->disassemble) . '<br>';
					$newline = collapse_repeated($newline, htmlspecialchars($row->disassemble), false);
				}
				$mapping[$lineNumber] [] = $newline;
			}
			$mapping[$lineNumber] [] = collapse_repeated('', '', true);
		}
		foreach ($resulttypes as $value) {
			$maxFehlerMapping[$lineNumber][$value] = $maxFehler[$value];
		}
	}

	while ($row = mysqli_fetch_object($highlevelCode)) {
		// FIXME id unique?
		$content .= '<span data-line="' . $row->linenumber . '" class="sourcecode">' . $row->linenumber . ' : ' . $row->line . '</span><br>';
		if (array_key_exists($row->linenumber, $mapping)) {
			$content .= '<div class="mapping">';
			foreach ($mapping[$row->linenumber] as $index => $span) {
				$content .= $span;
			}
			$content .= '</div><div class="maxFehlerMapping"';
			foreach ($resulttypes as $value) {
					$content .= $value . '="' . $maxFehlerMapping[$row->linenumber][$value] . '" ';
			}
			$content .= '></div>';
		}
	}

	echo json_encode($content);
}

function getResulttypes(&$resulttypes)
{
	$abfrage = "SELECT resulttype FROM " . $GLOBALS['result_table'] . " GROUP BY resulttype;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	while ($row = mysqli_fetch_object($ergebnis)) {
		//echo $row->resulttype;
		array_push($resulttypes, $row->resulttype);
	}
}

function getResulttypesOUT()
{
	$resulttypes = array();

	$abfrage = "SELECT resulttype FROM " . $GLOBALS['result_table'] . " GROUP BY resulttype;";

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

	while ($row = mysqli_fetch_object($ergebnis)) {
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

	$ergebnis = mysqli_query($GLOBALS['db'], $abfrage);

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
	$numfields=mysqli_num_fields($ergebnis);

	for ($i=0; $i < $numfields; $i++) {
		$fieldInfo = mysqli_fetch_field_direct($ergebnis, $i);
		$fieldname[$i] = $fieldInfo->name;
	}

	for ($i=2; $i < $numfields; $i++) {
		$results["max"][$fieldname[$i]] = 0;
	}

	$maxFehler = 0;
	while ($row = mysqli_fetch_object($ergebnis)) {
		if ($version != 'latestip') {
			if ($row->instr_absolute != NULL) {
				$results["Daten"][$row->instr_absolute] = array();
				for ($i = 2; $i < $numfields ; $i++) {
					$fn = $fieldname[$i];
					$results["Daten"][$row->instr_absolute][$fn] = $row->$fn;

					if ($row->$fn > $results["max"][$fn]) {
						$results["max"][$fn] = $row->$fn;
					}
				}
			}
		} else {
			if ($row->latest_ip != NULL) {
				$results["Daten"][$row->latest_ip] = array();
				for ($i = 0 ; $i < $numfields ; $i++) {
					$fn = $fieldname[$i];
					$results["Daten"][$row->latest_ip][$fn] = $row->$fn;

					if ($row->$fn > $results["max"][$fn]) {
						$results["max"][$fn] = $row->$fn;
					}
				}
			}
		}
	}

	return $results;
}

?>
