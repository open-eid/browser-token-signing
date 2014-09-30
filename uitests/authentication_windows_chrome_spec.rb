#!/usr/bin/spec -cfn

require 'rubygems'
require 'win32ole'
require 'watir-webdriver'
require File.dirname(__FILE__) + '/windows_helpers.rb'
require File.dirname(__FILE__) + '/authentication_shared_win.rb'

describe "Authentication in Firefox on Windows" do
	it_should_behave_like "Authentication in browser Win"

  def open_browser
		open_chrome
  end
	
  def enter_pin pin
    sleep 1
    puts "Alustame PIN1 akna ootamist"
    
    wait_for_window "Parooli sisestamine on vajalik"
    ret = @autoit.WinWait("Parooli sisestamine on vajalik", '', 5)
    puts "Aken leitud #{ret}"
   
    if(ret==1)	    
        @autoit.WinActivate("Parooli sisestamine on vajalik")
        @autoit.Send(pin)
		hit_return
     end
   end
end