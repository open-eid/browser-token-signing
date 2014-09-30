#!/usr/bin/spec -cfn

require 'rubygems'
require 'watir-webdriver'
require File.dirname(__FILE__) + '/linux_helpers.rb'
require File.dirname(__FILE__) + '/plugin_shared.rb'

describe 'Linux ID-card browser plugin' do
	it_should_behave_like "ID-card browser plugin"

  def text_area_content id
    @browser.textarea(:id=>id).value
  end

end
