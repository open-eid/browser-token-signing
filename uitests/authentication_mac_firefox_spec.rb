#!/usr/bin/spec -cfn

require 'rubygems'
require 'watir-webdriver'
require File.dirname(__FILE__) + '/mac_helpers.rb'
require File.dirname(__FILE__) + '/authentication_shared.rb'

describe "Authentication in Firefox on Mac" do
	it_should_behave_like "Authentication in browser"

  def open_browser
		open_firefox
  end

end