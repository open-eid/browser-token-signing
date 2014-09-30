#!/usr/bin/spec -cfn

require 'rubygems'
require 'watir-webdriver'
require 'win32ole'
require 'iconv'
require File.dirname(__FILE__) + '/windows_helpers.rb'
require File.dirname(__FILE__) + '/plugin_shared.rb'

describe 'Windows Firefox ID-card browser plugin' do
  it_should_behave_like "ID-card browser plugin"

  def open_browser
    open_firefox
  end

  def text_area_content id
    @browser.textarea(:id => id).value
  end

  def enter_pin pin
    pin_window_title = "Windows Security"

    wait_for_window pin_window_title
    ret = @autoit.WinWait(pin_window_title, '', 5)
    puts "1"
    if (ret==1)
      puts "2"
      @autoit.WinActivate(pin_window_title)
      @autoit.ControlSend(pin_window_title, "", 'Edit1', pin)
      @autoit.ControlClick(pin_window_title, '', 'OK')
    end
  end

  def close_pin2_blocked_message_window
    sleep 1
    pin2_blocked_message_window = RAutomation::Window.new(:title => "Viga")
    pin2_blocked_message_window.button(:value => "Ok").click
  end

  def hit_return_if_windows_else_do_nothing
    sleep 1
    puts "hit_return_if_windows_else_do_nothing"
    invalid_pin2_message_window = RAutomation::Window.new(:title => "Error validating PIN")
    puts invalid_pin2_message_window.button(:value => "OK").click
  end

end
