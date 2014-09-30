#!/usr/bin/spec -cfn

require 'safariwatir'
require File.dirname(__FILE__) + '/mac_helpers.rb'
require File.dirname(__FILE__) + '/plugin_shared.rb'

describe 'Mac Safari ID-card browser plugin' do
	it_should_behave_like "ID-card browser plugin"

  def open_browser
		open_safari
  end

  def text_area_content id
    @browser.text_area(:id, id).getContents
  end

end
