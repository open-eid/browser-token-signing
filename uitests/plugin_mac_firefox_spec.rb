#!/usr/bin/spec -cfn

require 'watir-webdriver'
require File.dirname(__FILE__) + '/mac_helpers.rb'
require File.dirname(__FILE__) + '/plugin_shared.rb'

describe 'Mac Firefox ID-card browser plugin' do
	it_should_behave_like "ID-card browser plugin"

  def open_browser
		open_firefox
  end
  
  def text_area_content id
    @browser.textarea(:id=>id).value
  end

end
