#!/usr/bin/spec -cfn

require 'rubygems'
require 'watir-webdriver'
require 'win32ole'
require File.dirname(__FILE__) + '/windows_helpers.rb'
require File.dirname(__FILE__) + '/plugin_shared.rb'

describe 'Windows IE ID-card browser plugin' do
	it_should_behave_like "ID-card browser plugin"

  def open_browser
		open_ie    
  end
  
  def text_area_content id
    @browser.textarea(:id=>id).value
  end
  
  def enter_pin pin
    sleep 5
    wait_for_window "  Windows Security"
    ret = @autoit.WinWait("  Windows Security", '', 5)
    if(ret==1)
        @autoit.WinActivate('  Windows Security')
        @autoit.Send(pin)
        @autoit.ControlClick("  Windows Security",'', 'OK')
      # else
     ## irb
     end
  end

  def hit_return_if_windows_else_do_nothing
    hit_return
  end

end
