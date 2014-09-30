#!/usr/bin/spec -cfn

require 'rubygems'
require 'watir-webdriver'
require File.dirname(__FILE__) + '/linux_helpers.rb'
require File.dirname(__FILE__) + '/authentication_shared.rb'

describe "Authentication in browser on Linux" do
	it_should_behave_like "Authentication in browser"
end