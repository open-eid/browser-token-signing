#!/usr/bin/spec -cfn

require 'rubygems'
require 'watir-webdriver'
require 'win32ole'
require File.dirname(__FILE__) + '/windows_helpers.rb'
require File.dirname(__FILE__) + '/plugin_shared.rb'

describe 'Windows Chrome ID-card browser plugin' do
	it_should_behave_like "ID-card browser plugin"

  def open_browser
		open_chrome
  end
  
  def text_area_content id
    @browser.textarea(:id=>id).value
  end

  def enter_pin pin
    sleep 5
    puts "Alustame"
    
    wait_for_window "EstEID PIN2"
    ret = @autoit.WinWait("EstEID PIN2", '', 5)
    puts "Aken leitud #{ret}"
   
    if(ret==1)
        @autoit.WinActivate('EstEID PIN2')

        # Focus to input box
        @autoit.Send("{TAB}")
        @autoit.Send(pin)
        @autoit.ControlClick("EstEID PIN2",'', 'OK')     
     end
  end

end
