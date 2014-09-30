#!/usr/bin/spec -cfn

require 'rubygems'
require 'win32ole'
require 'watir-webdriver'
require File.dirname(__FILE__) + '/windows_helpers.rb'
require File.dirname(__FILE__) + '/authentication_shared_win.rb'

describe "Authentication in Firefox on Windows" do
  it_should_behave_like "Authentication in browser Win"

  def open_browser
    open_firefox
  end

  def enter_pin pin
    sleep 1
    puts "Alustame PIN1 akna ootamist"

    wait_for_window @password_dialog_title
    ret = @autoit.WinWait(@password_dialog_title, '', 5)
    puts "Aken leitud #{ret}"

    if (ret==1)
      @autoit.WinActivate(@password_dialog_title)
      @autoit.Send(pin)
      hit_return
    end
  end
end