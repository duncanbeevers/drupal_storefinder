<?php

function storefinder_view_build() {
	drupal_add_css(drupal_get_path('module', 'storefinder') .'/storefinder.css');	
	
	$output = '<div id="storefinder_view">' . drupal_get_form('_storefinder_view_form') . '</div>';
	
	return $output;
}

function _storefinder_view_form() {
	$form = array();
  $form['address'] = array(
    '#type' => 'textfield', 
    '#title' => t('Your Address'),
    '#size' => 50, 
    '#maxlength' => 255,
    '#weight' => -9,
  );	
  $form['zip'] = array(
    '#type' => 'textfield', 
    '#title' => t('Your Zip Code'),
    '#size' => 50, 
    '#maxlength' => 255,
		'#required' => true,
    '#weight' => -6,
  );	
  $form['range'] = array(
    '#type' => 'select',
    '#title' => t('Distance'),
    '#options' => drupal_map_assoc(array(25,50,75,100)),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Find Locations',
    '#weight' => 10,
  );

	
	return $form;
}

function _storefinder_view_form_submit($form_id, $form_values) {
	$query = 'r:' . $form_values['range'] . ' z:' . $form_values['zip'] . ' a:' . $form_values['address'];

	return "locations/map/" . $query;
}

function storefinder_map_view() {
	drupal_add_css(drupal_get_path('module', 'storefinder') .'/storefinder.css');

	$key = variable_get('sf_googlemap_api_key', '');
	if ($key !='') {
		drupal_set_html_head('<script type="text/javascript" src="http://maps.google.com/maps?file=api&amp;v=2&amp;key=' . $key . '"></script>');
		
		$query = str_replace(' a:','|',str_replace(' z:','|',str_replace('r:','',urldecode(arg(2)))));
		list($range, $zip, $addr) = explode("|", $query);
		
		$gmap = "http://maps.google.com/maps/geo?q=$zip&output=xml&key=$key";
		$gmappage = file_get_contents($gmap);
		$xml = new SimpleXMLElement($gmappage);
		list($longitude, $latitude, $altitude) = explode(",", $xml->Response->Placemark->Point->coordinates);
			
		//todo: optomize
		$pagesize = 10;
                $distance_sql = sprintf(
                  "( 3959 * acos( cos( radians('%f') ) * cos( radians( lat ) ) ".
                  "* cos( radians( lon ) - radians('%f') ) + sin( radians('%f') ) ".
                  "* sin( radians( lat ) ) ) ) AS distance", $latitude, $longitude, $latitude);
		$sql = sprintf("SELECT *," . $distance_sql . " FROM {storefinder} HAVING distance < '%d' ORDER BY distance", $range);
		$count_sql = sprintf("SELECT COUNT(*), $distance_sql FROM {storefinder} HAVING distance < '%d'", $range);
		$dbresult = pager_query($sql, $pagesize, 0, $count_sql);

		$mapmarks = '';
		$i = 0;
		while ($row = @mysql_fetch_object($dbresult)){
			$mapmarks .= "map.addOverlay(createMarker(new GLatLng(" . $row->lat . ", " . $row->lon . "), " .
					$i . ", " .
					json_encode(
						'<span id = \"StoreTitle\">' .
						htmlentities($row->sname) .
						"</span><br />" .
						htmlentities($row->address) .
						"<br />" .
						htmlentities($row->city) . ", " .
						htmlentities($row->state) . " " .
						htmlentities($row->zip) . "<br />" .
						htmlentities($row->phone)
					) . "));
			bounds.extend(new GLatLng(" . $row->lat . ", " . $row->lon . "));";
			$i += 1;
		}
		mysql_data_seek($dbresult, 0);
		
		$output .= "<script type=\"text/javascript\">
									//<![CDATA[
									$(document).ready(function() {
										if (GBrowserIsCompatible()) {
											var bounds = new GLatLngBounds();
											var map = new GMap2(document.getElementById(\"storefinder_map\"));
											map.addControl(new GLargeMapControl());
											map.setCenter(new GLatLng(" . $latitude . ", " . $longitude . "), 11);
											
							        var baseIcon = new GIcon();
											baseIcon.shadow = \"" . base_path() . drupal_get_path('module', 'storefinder') . "/markers/shadow.png\";
											baseIcon.iconSize = new GSize(20, 34);
											baseIcon.shadowSize = new GSize(37, 34);
											baseIcon.iconAnchor = new GPoint(9, 34);
											baseIcon.infoWindowAnchor = new GPoint(9, 2);
											baseIcon.infoShadowAnchor = new GPoint(18, 25);
							
											// Creates a marker whose info window displays the letter corresponding
											// to the given index.
											function createMarker(point, index, text) {
												// Create a lettered icon for this point using our icon class
												var letter = String.fromCharCode(\"A\".charCodeAt(0) + index);
												var letteredIcon = new GIcon(baseIcon);
												letteredIcon.image = \"" . base_path() . drupal_get_path('module', 'storefinder') . "/markers/marker\" + letter + \".png\";
							
												// Set up our GMarkerOptions object
												markerOptions = { icon:letteredIcon };
												var marker = new GMarker(point, markerOptions);
							
												GEvent.addListener(marker, \"click\", function() {
													marker.openInfoWindowHtml(\"<div id='MarkerInfo'>\" + text + \"</div>\");
												});
												return marker;
											}
											" . $mapmarks . "
											
											map.setZoom(map.getBoundsZoomLevel(bounds));
						          map.setCenter(bounds.getCenter());
										}
									});  
									//]]>
								</script>";
		$output .= '<div id="storefinder_map" style="width: 590px; height: 425px; margin: 0px auto;"></div>';
		$output .= '<div id="spacer"><p>&nbsp;</p></div>';
			
		//list the locations below
	
		$output .= '<div id="storefinder_list">';
		$output .= tapir_get_table('storefinder_list_table', $dbresult);
		$output .= '</div>';

		$output .= '<div id="storefinder_pager">';
		$output .= theme('pager', $pagesize);
		$output .= '</div>';
		
		return $output;
	}
	else
	{
		drupal_set_message(t('This site does not have its api key setup.'));
	}
}

function storefinder_list_table($op, $result) {
	$query = str_replace(' a:','|',str_replace(' z:','|',str_replace('r:','',urldecode(arg(2)))));
	list($range, $zip, $addr) = explode("|", $query);
	$i = 0;

  switch ($op) {
    case 'fields':
			$fields[] = array('name' => 'MarkerIcon', 'title' => '', 'weight'  => 2, 'enabled' => TRUE, 'style' => 'width: 75px; text-align: center;');
      $fields[] = array('name' => 'StoreName', 'title' => t('Store Name'), 'weight' => 2, 'enabled' => TRUE, 'style' => 'width: 40%;');
      $fields[] = array('name' => 'Address', 'title' => t('Address'), 'weight' => 3,'enabled' => TRUE);
      return $fields;

  	case 'data':
		while ($row = db_fetch_object($result)) {
			if (!empty($row->http)) {
				$httpl = '<a href="' . $row->http . '" target="_blank">Visit</a>&nbsp;&nbsp;&nbsp;';
			}
			$data['MarkerIcon'][] = '<div align="center"><img src="' . base_path() . drupal_get_path('module', 'storefinder') . '/markers/marker' .chr(65 + $i) . '.png" /><br />' . round($row->distance, 2) . ' Miles</div>';
	  	$data['StoreName'][] = $row->sname . '<br /><br />'  . $row->phone . '<br />' . $httpl . '<a href="http://maps.google.com/maps?f=q&hl=en&q=from:+' . $addr . ', ' . $zip . '+to:+' . $row->address . ',+' . $row->city . ',+' . $row->state . '+ ' . $row->zip . '" target="_blank">Directions</a>';
		  $data['Address'][] = $row->address . '<br />' . $row->city . ', ' . $row->state . '<br />' . $row->zip;
			
			$data['#attributes'][] = array('valign' => 'top');
			$i += 1;
		}
   	return $data;
	}
}
