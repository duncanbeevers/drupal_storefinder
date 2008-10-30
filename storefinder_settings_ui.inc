<?php

function storefinder_admin_settings() {
  //note the name of the variable for center of the map is latlong although the format is actually longitude, latitude

  $form['sf_googlemap_api_key'] = array(
    '#type' => 'textfield', 
    '#title' => t('Google map API key'),
    '#default_value' => variable_get('sf_googlemap_api_key', ''), 
    '#size' => 50, 
    '#maxlength' => 255,
    '#description' => t('Your personal Googlemaps API key.  You must get this for each separate website at <a href="http://www.google.com/apis/maps/">Google Map API website</a>.'),
  );

  $form['sf_googlemap_center'] = array(
    '#type' => 'textfield',
    '#title' => t('Default center'),
    '#default_value' => variable_get('sf_googlemap_center', ''),
    '#size' => 50,
    '#maxlength' => 255,
    '#description' => t('The default center coordinates of Google map, expressed as a decimal latitude and longitude, separated by a comma.'),
  );
  $form['sf_googlemap_zoom'] = array(
    '#type' => 'select',
    '#title' => t('Default zoom'),
    '#default_value' => variable_get('sf_googlemap_zoom', ''),
    '#options' => drupal_map_assoc(range(0, 17)),
    '#description' => t('The default zoom level of a Google map.'),
  );
	
	
	return system_settings_form($form);
}
