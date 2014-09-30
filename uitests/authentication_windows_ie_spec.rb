#!/usr/bin/spec -cfn

require 'rubygems'
# gem 'watir-webdriver', '>=0.1.8'
require 'watir-webdriver'
require 'win32ole'
require File.dirname(__FILE__) + '/windows_helpers.rb'
require File.dirname(__FILE__) + '/authentication_shared_ie.rb'

describe "Authentication in Internet Explorer on Windows" do
	it_should_behave_like "Authentication in browser Win"

  def open_browser
		open_ie
  end

  def enter_pin pin
    sleep 5
    puts "Alustame"
    #wait_for_window "  Windows Security"
    wait_for_window "Windows Security"
    ret = @autoit.WinWait("Windows Security", '', 2)
    puts "Ret tulemus #{ret}"
    if(ret==1)
        @autoit.WinActivate('Windows Security')
        hit_return
        ret=0
        wait_for_window "  Windows Security"
        ret = @autoit.WinWait("  Windows Security", '', 2)
        if(ret==1)
          @autoit.WinActivate("  Windows Security")
          @autoit.Send(pin)
          @autoit.ControlClick("  Windows Security",'', 'OK')
        end
        
     end
  end
end