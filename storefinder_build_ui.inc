<?php

function storefinder_admin_build() {
	drupal_add_css(drupal_get_path('module', 'storefinder') .'/storefinder.css');
	
	//will manage the locations via this page
	$pagesize = 25;

	$sql = "SELECT s.* FROM {storefinder} s ORDER BY s.sname";
	$dbresult	= pager_query($sql, $pagesize);
	
	$output = '<div id="storefinder_content_table">';
	$output .= tapir_get_table('storefinder_content_table', $dbresult);
	$output .= '</div>';

	$output .= '<div id="storefinder_pager">';
	$output .= theme('pager', $pagesize);
	$output .= '</div>';
	
	return $output;
}

function storefinder_content_table($op, $result) {
  switch ($op) {
    case 'fields':
      $fields[] = array('name' => 'actions', 'title' => t('&nbsp;'), 'weight' => 1, 'enabled' => TRUE, 'style' => 'width: 65px;text-align: center;');
      $fields[] = array('name' => 'StoreName', 'title' => t('Store Name'), 'weight' => 2, 'enabled' => TRUE, 'style' => 'width: 40%;');
      $fields[] = array('name' => 'Address', 'title' => t('Address'), 'weight' => 3,'enabled' => TRUE);
      return $fields;

  	case 'data':
		while ($row = db_fetch_object($result)) {
		
	  	$data['actions'][] = '<a href="' . base_path() . 'admin/content/slocations/' . $row->sid . '/edit"><img src="' . base_path() . drupal_get_path('module', 'storefinder') . '/images/edit.gif"></a>&nbsp;<a href="' . base_path() . 'admin/content/slocations/' . $row->sid . '/delete"><img src="' . base_path() . drupal_get_path('module', 'storefinder') . '/images/delete.gif" onclick="return confirm(\'Are you sure you want to delete the following store: ' . $row->sname . '?\');">';
	  	$data['StoreName'][] = $row->sname . '<br /><br/>' . $row->http;
		  $data['Address'][] = $row->address . '<br />' . $row->city . ', ' . $row->state . '<br />' . $row->zip . '<br /><br />' . $row->phone . '<br /><br />' . $row->lat . ', ' . $row->lon;
			
			$data['#attributes'][] = array('valign' => 'top');
		}
   	return $data;
	}
}

function storefinder_location_form($dvalues = NULL) {
	$form = array();
  $form['storeName'] = array(
    '#type' => 'textfield', 
    '#title' => t('Store Name'),
    '#default_value' => $dvalues->sname, 
    '#size' => 50, 
    '#maxlength' => 255,
		'#required' => true,
    '#weight' => -10,
  );	
  $form['address'] = array(
    '#type' => 'textfield', 
    '#title' => t('Store Address'),
    '#default_value' => $dvalues->address, 
    '#size' => 50, 
    '#maxlength' => 255,
		'#required' => true,
    '#weight' => -9,
  );	
  $form['city'] = array(
    '#type' => 'textfield', 
    '#title' => t('Store City'),
    '#default_value' => $dvalues->city, 
    '#size' => 50, 
    '#maxlength' => 255,
		'#required' => true,
    '#weight' => -8,
  );	
  $form['state'] = array(
    '#type' => 'textfield', 
    '#title' => t('Store State'),
    '#default_value' => $dvalues->state, 
    '#size' => 50, 
    '#maxlength' => 255,
		'#required' => true,
    '#weight' => -7,
  );	
  $form['zip'] = array(
    '#type' => 'textfield', 
    '#title' => t('Store Zip Code'),
    '#default_value' => $dvalues->zip, 
    '#size' => 50, 
    '#maxlength' => 255,
		'#required' => true,
    '#weight' => -6,
  );	
  $form['country'] = array(
    '#type' => 'textfield', 
    '#title' => t('Store Country'),
    '#default_value' => (empty($dvalues->country)) ? 'USA' :  $dvalues->country, 
    '#size' => 50, 
    '#maxlength' => 255,
    '#weight' => -5,
  );
  $form['phone'] = array(
    '#type' => 'textfield', 
    '#title' => t('Store Phone'),
    '#default_value' => $dvalues->phone, 
    '#size' => 50, 
    '#maxlength' => 255,
    '#weight' => -4,
  );	
  $form['httpl'] = array(
    '#type' => 'textfield', 
		'#description' => t('Please enter addresses in the following format http://www.sitename.com/'),
    '#title' => t('Website'),
    '#default_value' => $dvalues->http, 
    '#size' => 50, 
    '#maxlength' => 255,
    '#weight' => -3,
  );
  $form['lat'] = array(
    '#type' => 'textfield', 
		'#description' => t('Will be found based on address if not supplied.'),
    '#title' => t('Latitude'),
    '#default_value' => $dvalues->lat, 
    '#size' => 50, 
    '#maxlength' => 255,
    '#weight' => 8,
  );	
  $form['lon'] = array(
    '#type' => 'textfield', 
		'#description' => t('Will be found based on address if not supplied.'),
    '#title' => t('Longitude'),
    '#default_value' => $dvalues->lon, 
    '#size' => 50, 
    '#maxlength' => 255,
    '#weight' => 9,
  );
	$act = 'add';
	$btn = 'Add Store';
	if (isset($dvalues)) {
		$act = 'edit';
		$btn = 'Update Store';
	}
  $form['action'] = array(
    '#type' => 'hidden',
    '#value' => $act,
    '#weight' => 10,
  );
  $form['store'] = array(
    '#type' => 'hidden',
    '#value' => $dvalues->sid,
    '#weight' => 10,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $btn,
    '#weight' => 10,
  );
	
	return $form;
}

function storefinder_location_form_submit($form_id, $form_values) {
	$key = variable_get('sf_googlemap_api_key', '');
	$action = $form_values['action'];
	if ($key != '') {
		$store = $form_values['store'];
		$sname = $form_values['storeName'];
		$saddr = $form_values['address'];
		$scity = $form_values['city'];
		$sstate = $form_values['state'];
		$szip = $form_values['zip'];
		$scountry = $form_values['country'];
		$sphone = $form_values['phone'];
		$slat = $form_values['lat'];
		$slon = $form_values['lon'];
		$httpl = $form_values['httpl'];
		
		if ($slat == '' || $slon == '') {
			// sample query 604+Moore+Rd,+Avon+Lake,+Ohio
			$query = str_replace(' ', '+', $saddr . ', ' . $scity .', ' . $sstate);
			$gmap = "http://maps.google.com/maps/geo?q=$query&output=xml&key=$key";
			
			$gmappage = file_get_contents($gmap);
			$xml = new SimpleXMLElement($gmappage);
			list($longitude, $latitude, $altitude) = explode(",", $xml->Response->Placemark->Point->coordinates);
			$slat = $latitude;
			$slon = $longitude;
		}
		
		if ($action == 'add') {
			$sql = "insert into {storefinder} (sname, address, city, state, zip, country, phone, http, lat, lon)";
			$sql .= "values('%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%f', '%f');";
			db_query($sql, $sname, $saddr, $scity, $sstate, $szip, $scountry, $sphone, $httpl, $slat, $slon);
			
			return "admin/content/slocations";
			drupal_set_message(t('The new store has been added.'));
		}
		elseif ($action == 'edit') {
			$sql = "update {storefinder} set sname = '%s', address = '%s', city = '%s', state = '%s', zip = '%s', country = '%s', phone = '%s', http = '%s', lat  = '%f', lon = '%f' where sid = '%d';";
			db_query($sql, $sname, $saddr, $scity, $sstate, $szip, $scountry, $sphone, $httpl, $slat, $slon, $store);
			
			return "admin/content/slocations";
			drupal_set_message(t('The new store has been added.'));
		}
	}
	else
	{
		return "admin/content/slocations";
		drupal_set_message(t('Your api key is not setup.'));
	}
}

function storefinder_location_action() {
	$sid = arg(3);
	$action = arg(4);
	
	switch ($action) {
		case 'edit':
			$dbresult = db_query("select * from {storefinder} where sid = %d", $sid);
			$dbvalues = mysql_fetch_object($dbresult);
			$content = drupal_get_form('storefinder_location_form',$dbvalues);
			break;
		case 'delete':
			_storefinder_location_delete($sid);
			break;
	}	
	
	return $content;
}

function _storefinder_location_delete($sid) {

  db_query('DELETE FROM {storefinder} WHERE sid = %d', $sid);

  drupal_set_message(t("Store Location Deleted"));
  drupal_goto('admin/content/slocations');
}